#include <gtest/gtest.h>

#include <Z/Zaban/Langs/CLang/Lexer.hpp>
#include <Z/Zaban/Langs/CLang/Parser.hpp>
#include <Z/Zaban/Langs/CLang/Preprocessor.hpp>
#include <cstddef>
#include <utility>

#include "Z/Zaban/AST/Declaration.hpp"
#include "Z/Zaban/AST/Declarations/LetDeclaration.hpp"
#include "Z/Zaban/AST/Expressions/BinaryExpression.hpp"
#include "Z/Zaban/AST/Expressions/FunctionExpression.hpp"
#include "Z/Zaban/AST/Node.hpp"
#include "Z/Zaban/AST/Statements/BlockStatement.hpp"
#include "Z/Zaban/AST/Statements/FlowStatement.hpp"
#include "Z/Zaban/Langs/CLang/AST/Annotations/PrimitiveAnnotation.hpp"
#include "Z/Zaban/Langs/CLang/ParserDiagnostic.hpp"
#include "Z/Zaban/Lex/LexerDiagnostic.hpp"
#include "gtest/gtest.h"

namespace Z::Zaban::Tests {
    using namespace Z::Zaban::Langs::CLang;
    using namespace Z::Zaban::AST;
    namespace {
        struct Harness {
            CModule                  module_;
            CParserDiagnosticContext diags_;

            std::vector<CLexerDiagnostic> lex_diags_;
            std::vector<PPDiagnostic>     pp_diags_;
            CPpErrorFlags                 pp_errors_ = CPpErrorFlags::None;

            explicit Harness(std::string_view src) {
                CLexerBufferType buffer = src;
                CLexer           lexer(buffer);
                lexer.scan();
                auto tokens = lexer.finalize();
                lex_diags_ =
                    static_cast<CLexerDiagnosticContext&>(lexer.diagnostics())
                        .all();

                CPreprocessor pp(buffer);
                auto          stream = pp.process(std::move(tokens));
                pp_diags_            = pp.diagnostics();
                pp_errors_           = pp.errors();

                CParser parser{std::move(stream)};
                module_ = parser.parse();
                diags_  = parser.diagnostics();
            }

            void print_module() const {
                for (std::size_t i = 0; i < module_.size(); ++i) {
                    const auto& mode = module_[i];
                    if (!mode) {
                        std::cout << "[" << i << "]" << " nullptr\n";
                        continue;
                    }
                    const auto loc = mode->location();
                    std::cout
                        << "[" << i
                        << "] kind = " << std::to_underlying(mode->node_kind())
                        << " location: [" << loc.begin << ", " << loc.end
                        << ")\n";
                }
            }

            /// checks node kind given the index of the node in the module and
            /// the kind we expect it to be
            ::testing::AssertionResult check_kind(
                const std::size_t idx, Zaban::AST::NodeKind kind) const {
                if (idx >= module_.size()) {
                    return ::testing::AssertionFailure()
                           << "Node index " << idx
                           << " is out of bounds (module size: "
                           << module_.size() << ')';
                }
                const auto& node = module_[idx];
                if (!node) {
                    return ::testing::AssertionFailure()
                           << "Node at index " << idx << " is a nullptr\n";
                }

                if (node->node_kind() == kind) {
                    return ::testing::AssertionSuccess();
                }

                return ::testing::AssertionFailure()
                       << "Node at index" << idx
                       << ": expected kind=" << std::to_underlying(kind)
                       << " got kind=" << std::to_underlying(node->node_kind())
                       << "\n";
            }

            /// lists every lexer/preprocessor error on failure.
            ::testing::AssertionResult upstream_clean() const {
                const auto is_error = [](const auto& d) {
                    return d.severity() == Lex::LexerDiagnosticSeverity::Error;
                };
                const bool lex_failed =
                    std::ranges::any_of(lex_diags_, is_error);
                // the flags and the diagnostic list are separate records, so
                // check both rather than trust one to mirror the other.
                const bool pp_failed =
                    pp_errors_ != CPpErrorFlags::None ||
                    std::ranges::any_of(pp_diags_, [](const PPDiagnostic& d) {
                        return d.severity ==
                               Lex::LexerDiagnosticSeverity::Error;
                    });
                if (!lex_failed && !pp_failed) {
                    return ::testing::AssertionSuccess();
                }

                auto failure = ::testing::AssertionFailure();
                for (const auto& d: lex_diags_) {
                    if (!is_error(d)) continue;
                    failure << "\n  lexer: kind "
                            << std::to_underlying(d.kind()) << " at ["
                            << d.range().begin << ", " << d.range().end << ")";
                }
                for (const auto& d: pp_diags_) {
                    if (d.severity != Lex::LexerDiagnosticSeverity::Error)
                        continue;
                    failure << "\n  preprocessor: code "
                            << std::to_underlying(d.code) << " at ["
                            << d.range.begin << ", " << d.range.end << ")";
                    if (!d.arg.empty()) failure << " '" << d.arg << "'";
                }
                if (pp_errors_ != CPpErrorFlags::None) {
                    failure << "\n  preprocessor flags: "
                            << std::to_underlying(pp_errors_);
                }
                return failure;
            }
            ::testing::AssertionResult parser_clean() const {
                if (!diags_.has_errors()) return testing::AssertionSuccess();

                auto failure = ::testing::AssertionFailure();
                for (const auto& d: diags_.all()) {
                    if (d.severity() != Lex::LexerDiagnosticSeverity::Error) {
                        continue;
                    }
                    failure << "\n  parser: kind "
                            << std::to_underlying(d.kind()) << " at ["
                            << d.range().begin << ", " << d.range().end << ")";
                }
                return failure;
            }
            std::vector<CParserDiagnostic> get_diags() const {
                return diags_.all();
            }
        };

    }  // namespace

    TEST(CParserTest, Spaces) {
        Harness h{"int main  () { return 0;}"};
        ASSERT_TRUE(h.upstream_clean());
        ASSERT_TRUE(h.parser_clean());
    }
    TEST(CParserTest, NoHang) {
        Harness h{"int main() { return 0; } // x"};
        ASSERT_TRUE(h.upstream_clean());
        ASSERT_TRUE(h.parser_clean());
    }

    TEST(CParserTest, ReturnTwo) {
        Harness h{"int main() { return 2; }"};
        ASSERT_TRUE(h.upstream_clean());
        ASSERT_TRUE(h.parser_clean());
    }
    TEST(CParserTest, MissingParen) {
        Harness h{"int main( {return 0;}"};
        ASSERT_TRUE(h.upstream_clean());
        ASSERT_FALSE(h.parser_clean());
        auto diags = h.get_diags();
        ASSERT_EQ(diags.size(), 1);
        ASSERT_EQ(diags.back().kind(),
                  CParserDiagnosticKind::ErrorExpectedToken);
    };

    TEST(CParserTest, MissingBrace) {
        Harness h{"int main() {return 0;"};
        ASSERT_TRUE(h.upstream_clean());
        ASSERT_FALSE(h.parser_clean());
        auto diags = h.get_diags();
        ASSERT_EQ(diags.size(), 1);
        ASSERT_EQ(diags.back().kind(),
                  CParserDiagnosticKind::ErrorExpectedToken);
    };

    TEST(CParserTest, MissingSemi) {
        Harness h{"int main() {return 0}"};
        ASSERT_TRUE(h.upstream_clean());
        ASSERT_FALSE(h.parser_clean());
        auto diags = h.get_diags();
        ASSERT_EQ(diags.size(), 1);
        ASSERT_EQ(diags.back().kind(),
                  CParserDiagnosticKind::ErrorExpectedToken);
    };

    TEST(CParserTest, WrongCase) {
        Harness h{"int main() {RETURN 0;}"};
        ASSERT_TRUE(h.upstream_clean());
        ASSERT_FALSE(h.parser_clean());
        auto diags = h.get_diags();
        ASSERT_EQ(diags.size(), 1);
        ASSERT_EQ(diags.back().kind(),
                  CParserDiagnosticKind::ErrorExpectedToken);
    };

    TEST(CParserTest, BitWise) {
        Harness h{"int main() { return ~2; }"};
        ASSERT_TRUE(h.upstream_clean());
        ASSERT_TRUE(h.parser_clean());
    }
    TEST(CParserTest, CompoundAssign) {
        Harness h{R"c(int main() {
  int x = 5;
  x += 3;          // 8
  x -= 1;          // 7
  x *= 2;          // 14
  int y = x++;     // y=14, x=15
  int z = ++x;     // z=16, x=16
  int b = 12 & 10; // 8
  b = b | 1;       // 9
  b = b ^ 3;       // 10
  b = b << 2;      // 40
  b = b >> 1;      // 20
  char c = 'A';
  c++; // 'B' = 66
  return (y == 14) + (z == 16) + (b == 20) + (c == 66);
}
)c"};
        ASSERT_TRUE(h.upstream_clean());
        ASSERT_TRUE(h.parser_clean());
        ASSERT_EQ(h.module_.size(), 1u);
    }

    TEST(CParserTest, Associativity) {
        Harness h{"int main() { return 6 / 2 / 3; }"};
        ASSERT_TRUE(h.upstream_clean());
        ASSERT_TRUE(h.parser_clean());
        ASSERT_EQ(h.module_.size(), 1);
        h.print_module();
    }

    TEST(CParserTest, TwoModules) {
        Harness h{
            R"c(
              int main() { return (-12) / 2; }
              int prec(int a, int b) {
                 return 2 + 3 * 4;
              })c"};
        ASSERT_TRUE(h.upstream_clean());
        ASSERT_TRUE(h.parser_clean());
        ASSERT_EQ(h.module_.size(), 2);
        ASSERT_TRUE(h.check_kind(0, Zaban::AST::NodeKind::Declaration));
        ASSERT_TRUE(h.check_kind(1, Zaban::AST::NodeKind::Declaration));
        h.print_module();
    }

    TEST(CParserTest, SimpleDeclaration) {
        Harness h{"int x; "};
        ASSERT_TRUE(h.upstream_clean());
        ASSERT_TRUE(h.parser_clean());
        h.print_module();
        ASSERT_TRUE(h.check_kind(0, Zaban::AST::NodeKind::Declaration));
        ASSERT_EQ(h.module_[0]
                      ->cast<Zaban::AST::DeclarationNode<COffsetType>>()
                      ->kind(),
                  Zaban::AST::DeclarationKind::LetDecl);
    }

    TEST(CParserTest, GlobalFuncAndVar) {
        Harness h{
            R"c(
    int a;
    int b;
    int main() {
    a = b = 4;
    return a - b;
    }
)c"};
        ASSERT_TRUE(h.upstream_clean());
        ASSERT_TRUE(h.parser_clean());
        ASSERT_EQ(h.module_.size(), 3);
        h.print_module();
        ASSERT_TRUE(h.check_kind(0, Zaban::AST::NodeKind::Declaration));
        ASSERT_TRUE(h.check_kind(1, Zaban::AST::NodeKind::Declaration));
        ASSERT_TRUE(h.check_kind(2, Zaban::AST::NodeKind::Declaration));
        ASSERT_EQ(h.module_[0]
                      ->cast<Zaban::AST::DeclarationNode<COffsetType>>()
                      ->kind(),
                  Zaban::AST::DeclarationKind::LetDecl);
    }
    TEST(CParserTest, FunctionParameterBody) {
        Harness h{"int add(int a, int b) {return a + b;}"};

        ASSERT_TRUE(h.upstream_clean());
        ASSERT_TRUE(h.parser_clean());
        ASSERT_EQ(h.module_.size(), 1);

        ASSERT_NE(h.module_[0], nullptr);

        auto decl =
            h.module_[0]
                ->dyn_cast<
                    Zaban::AST::Declarations::LetDeclaration<COffsetType>>();
        ASSERT_NE(decl, nullptr);

        auto init = decl->get_initializer();
        ASSERT_NE(init, nullptr);

        auto fn =
            init->dyn_cast<Zaban::AST::FunctionExpressionNode<COffsetType>>();
        ASSERT_NE(fn, nullptr);
        ASSERT_EQ(fn->get_argc(), 2);

        auto first = fn->get_arg_at(0);
        ASSERT_NE(first, nullptr);
        ASSERT_NE(first->get_name(), nullptr);

        auto annot = first->get_annotation();
        ASSERT_NE(annot, nullptr);
        auto type = annot->dyn_cast<Langs::CLang::AST::Annotations::
                                        PrimitiveAnnotationNode<COffsetType>>();
        ASSERT_NE(type, nullptr);
        ASSERT_EQ(type->get_specifierc(), 1);
        ASSERT_EQ(type->get_specifier_at(0).kind, CTokenKind::Int);

        auto body = fn->get_body();
        ASSERT_NE(body, nullptr);
        auto block = body->dyn_cast<
            Zaban::AST::Statements::BlockStatement<COffsetType>>();
        ASSERT_NE(block, nullptr);
        ASSERT_EQ(block->statementc(), 1);

        auto stmt = block->get_statement(0);
        ASSERT_NE(stmt, nullptr);
        auto ret = stmt->dyn_cast<
            Zaban::AST::Statements::FlowStatement<COffsetType>>();

        ASSERT_NE(ret, nullptr);
        EXPECT_EQ(ret->get_type(), Zaban::AST::Statements::FlowKind::Return);

        auto expr = ret->get_expr();
        ASSERT_NE(expr, nullptr);
        auto sum = expr->dyn_cast<
            Zaban::AST::Expressions::BinaryExpression<COffsetType>>();
        ASSERT_NE(sum, nullptr);
        EXPECT_EQ(sum->get_op(), Zaban::AST::Expressions::BinaryOperator::Add);
        ASSERT_NE(sum->get_left(), nullptr);
        ASSERT_NE(sum->get_right(), nullptr);
    }

}  // namespace Z::Zaban::Tests
