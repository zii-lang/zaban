#include <gtest/gtest.h>

#include <Z/Zaban/Langs/CLang/Lexer.hpp>
#include <Z/Zaban/Langs/CLang/Parser.hpp>
#include <Z/Zaban/Langs/CLang/Preprocessor.hpp>
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <variant>

#include "Z/Zaban/AST/Annotations/ArrayAnnotation.hpp"
#include "Z/Zaban/AST/Annotations/PointerAnnotation.hpp"
#include "Z/Zaban/AST/Declaration.hpp"
#include "Z/Zaban/AST/Declarations/LetDeclaration.hpp"
#include "Z/Zaban/AST/Expressions/AssignmentExpression.hpp"
#include "Z/Zaban/AST/Expressions/BinaryExpression.hpp"
#include "Z/Zaban/AST/Expressions/CallAccessExpression.hpp"
#include "Z/Zaban/AST/Expressions/FunctionExpression.hpp"
#include "Z/Zaban/AST/Expressions/GroupExpression.hpp"
#include "Z/Zaban/AST/Expressions/IndexAccessExpression.hpp"
#include "Z/Zaban/AST/Expressions/PrefixExpression.hpp"
#include "Z/Zaban/AST/Expressions/SuffixExpression.hpp"
#include "Z/Zaban/AST/Node.hpp"
#include "Z/Zaban/AST/Statements/BlockStatement.hpp"
#include "Z/Zaban/AST/Statements/DeclarationStatement.hpp"
#include "Z/Zaban/AST/Statements/ExpressionStatement.hpp"
#include "Z/Zaban/AST/Statements/FlowStatement.hpp"
#include "Z/Zaban/Langs/CLang/AST/Annotations/EnumAnnotation.hpp"
#include "Z/Zaban/Langs/CLang/AST/Annotations/FunctionAnnotation.hpp"
#include "Z/Zaban/Langs/CLang/AST/Annotations/PrimitiveAnnotation.hpp"
#include "Z/Zaban/Langs/CLang/AST/Annotations/StructAnnotation.hpp"
#include "Z/Zaban/Langs/CLang/AST/Annotations/UnionAnnotation.hpp"
#include "Z/Zaban/Langs/CLang/AST/Declaration/DeclarationGroup.hpp"
#include "Z/Zaban/Langs/CLang/AST/Declaration/EnumDeclaration.hpp"
#include "Z/Zaban/Langs/CLang/AST/Declaration/StructDeclaration.hpp"
#include "Z/Zaban/Langs/CLang/AST/Declaration/UnionDeclaration.hpp"
#include "Z/Zaban/Langs/CLang/AST/Expressions/InitializerListExpression.hpp"
#include "Z/Zaban/Langs/CLang/AST/Expressions/MemberAccessExpression.hpp"
#include "Z/Zaban/Langs/CLang/AST/Expressions/TernaryExpression.hpp"
#include "Z/Zaban/Langs/CLang/AST/Statements/DoWhileStatement.hpp"
#include "Z/Zaban/Langs/CLang/AST/Statements/ForStatement.hpp"
#include "Z/Zaban/Langs/CLang/AST/Statements/IfStatement.hpp"
#include "Z/Zaban/Langs/CLang/AST/Statements/WhileStatement.hpp"
#include "Z/Zaban/Langs/CLang/ParserDiagnostic.hpp"
#include "Z/Zaban/Lex/LexerDiagnostic.hpp"

namespace Z::Zaban::Tests {
    using namespace Z::Zaban::Langs::CLang;
    using namespace Z::Zaban::AST;
    namespace {
        using Let        = Declarations::LetDeclaration<COffsetType>;
        using Function   = FunctionExpressionNode<COffsetType>;
        using Block      = Statements::BlockStatement<COffsetType>;
        using Flow       = Statements::FlowStatement<COffsetType>;
        using Binary     = Expressions::BinaryExpression<COffsetType>;
        using Assignment = Expressions::AssignmentExpression<COffsetType>;
        using Prefix     = Expressions::PrefixExpression<COffsetType>;
        using Suffix     = Expressions::SuffixExpression<COffsetType>;
        using Call       = Expressions::CallAccessExpression<COffsetType>;
        using Index      = Expressions::IndexAccessExpression<COffsetType>;
        using Group      = Expressions::GroupExpression<COffsetType>;
        using Array      = Annotations::ArrayAnnotation<COffsetType>;
        using Pointer    = Annotations::PointerAnnotation<COffsetType>;
        using FunctionType =
            Langs::CLang::AST::Annotations::FunctionAnnotationNode<COffsetType>;
        using DeclarationGroup =
            Langs::CLang::AST::Declaration::DeclarationGroupNode<COffsetType>;
        using Primitive =
            Langs::CLang::AST::Annotations::PrimitiveAnnotationNode<
                COffsetType>;
        using CStruct =
            Langs::CLang::AST::Annotations::StructAnnotationNode<COffsetType>;
        using CStructDecl =
            Langs::CLang::AST::Declaration::StructDeclarationNode<COffsetType>;
        using CUnion =
            Langs::CLang::AST::Annotations::UnionAnnotationNode<COffsetType>;
        using CUnionDecl =
            Langs::CLang::AST::Declaration::UnionDeclarationNode<COffsetType>;
        using CEnum =
            Langs::CLang::AST::Annotations::EnumAnnotationNode<COffsetType>;
        using CEnumDecl =
            Langs::CLang::AST::Declaration::EnumDeclarationNode<COffsetType>;
        using Ternary =
            Langs::CLang::AST::Expressions::TernaryExpressionNode<COffsetType>;
        using Member =
            Langs::CLang::AST::Expressions::MemberAccessExpressionNode<
                COffsetType>;
        using InitializerList =
            Langs::CLang::AST::Expressions::InitializerListExpressionNode<
                COffsetType>;
        using If = Langs::CLang::AST::Statements::IfStatementNode<COffsetType>;
        using While =
            Langs::CLang::AST::Statements::WhileStatementNode<COffsetType>;
        using DoWhile =
            Langs::CLang::AST::Statements::DoWhileStatementNode<COffsetType>;
        using For =
            Langs::CLang::AST::Statements::ForStatementNode<COffsetType>;

        struct Harness {
            std::string              source_;
            CModule                  module_;
            CParserDiagnosticContext diags_;

            std::vector<CLexerDiagnostic> lex_diags_;
            std::vector<PPDiagnostic>     pp_diags_;
            CPpErrorFlags                 pp_errors_ = CPpErrorFlags::None;

            explicit Harness(std::string_view src) : source_(src) {
                CLexerBufferType buffer = source_;
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

            std::shared_ptr<Let> declaration(std::size_t index = 0) const {
                if (index >= module_.size()) return nullptr;
                return std::dynamic_pointer_cast<Let>(module_[index]);
            }

            std::shared_ptr<Function> function(std::size_t index = 0) const {
                auto decl = declaration(index);
                return decl ? std::dynamic_pointer_cast<Function>(
                                  decl->get_initializer())
                            : nullptr;
            }

            std::shared_ptr<Block> body(std::size_t index = 0) const {
                auto fn = function(index);
                return fn ? std::dynamic_pointer_cast<Block>(fn->get_body())
                          : nullptr;
            }

            CExpression returned(std::size_t statement = 0) const {
                auto block = body();
                if (!block || statement >= block->statementc()) return nullptr;
                auto flow = std::dynamic_pointer_cast<Flow>(
                    block->get_statement(statement));
                return flow ? flow->get_expr() : nullptr;
            }

            template<typename T>
            std::string_view text(const std::shared_ptr<T>& node) const {
                if (!node) {
                    ADD_FAILURE() << "Cannot read the source of a null node";
                    return {};
                }
                auto range = node->location();
                if (range.begin > range.end || range.end > source_.size()) {
                    ADD_FAILURE() << "Invalid source range [" << range.begin
                                  << ", " << range.end << ')';
                    return {};
                }
                return std::string_view(source_).substr(
                    range.begin, range.end - range.begin);
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
        auto main =
            h.module_[0]
                ->dyn_cast<
                    Zaban::AST::Declarations::LetDeclaration<COffsetType>>();
        ASSERT_NE(main, nullptr);

        auto init = main->get_initializer();
        ASSERT_NE(init, nullptr);

        auto fn =
            init->dyn_cast<Zaban::AST::FunctionExpressionNode<COffsetType>>();
        ASSERT_NE(fn, nullptr);

        auto body = fn->get_body();
        ASSERT_NE(body, nullptr);

        auto block = body->dyn_cast<
            Zaban::AST::Statements::BlockStatement<COffsetType>>();
        ASSERT_NE(block, nullptr);

        auto stmt = block->get_statement(0);
        ASSERT_NE(stmt, nullptr);
        auto ret = stmt->dyn_cast<
            Zaban::AST::Statements::FlowStatement<COffsetType>>();
        ASSERT_NE(ret, nullptr);

        auto expr = ret->get_expr();
        ASSERT_NE(expr, nullptr);
        auto sum = expr->dyn_cast<
            Zaban::AST::Expressions::BinaryExpression<COffsetType>>();
        ASSERT_NE(sum, nullptr);

        auto left = sum->get_left();
        ASSERT_NE(left, nullptr);
        auto op = sum->get_op();
        ASSERT_EQ(op, Zaban::AST::Expressions::BinaryOperator::Div);
        auto right = sum->get_right();
        ASSERT_NE(right, nullptr);

        auto innerRight = left->dyn_cast<
            Zaban::AST::Expressions::BinaryExpression<COffsetType>>();
        ASSERT_NE(innerRight, nullptr);
        auto innerOp = innerRight->get_op();
        ASSERT_EQ(innerOp, Zaban::AST::Expressions::BinaryOperator::Div);
        EXPECT_EQ(h.text(innerRight->get_left()), "6");
        EXPECT_EQ(h.text(innerRight->get_right()), "2");
        EXPECT_EQ(h.text(right), "3");
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

        auto prec =
            h.module_[1]
                ->dyn_cast<
                    Zaban::AST::Declarations::LetDeclaration<COffsetType>>();
        ASSERT_NE(prec, nullptr);

        auto init = prec->get_initializer();
        ASSERT_NE(init, nullptr);

        auto fn =
            init->dyn_cast<Zaban::AST::FunctionExpressionNode<COffsetType>>();
        ASSERT_NE(fn, nullptr);

        auto body = fn->get_body();
        ASSERT_NE(body, nullptr);

        auto block = body->dyn_cast<
            Zaban::AST::Statements::BlockStatement<COffsetType>>();
        ASSERT_NE(block, nullptr);

        auto stmt = block->get_statement(0);
        ASSERT_NE(stmt, nullptr);
        auto ret = stmt->dyn_cast<
            Zaban::AST::Statements::FlowStatement<COffsetType>>();
        ASSERT_NE(ret, nullptr);

        auto expr = ret->get_expr();
        ASSERT_NE(expr, nullptr);
        auto sum = expr->dyn_cast<
            Zaban::AST::Expressions::BinaryExpression<COffsetType>>();
        ASSERT_NE(sum, nullptr);

        auto left = sum->get_left();
        ASSERT_NE(left, nullptr);
        auto op = sum->get_op();
        ASSERT_EQ(op, Zaban::AST::Expressions::BinaryOperator::Add);
        auto right = sum->get_right();
        ASSERT_NE(right, nullptr);

        auto innerRight = right->dyn_cast<
            Zaban::AST::Expressions::BinaryExpression<COffsetType>>();
        ASSERT_NE(innerRight, nullptr);
        auto innerOp = innerRight->get_op();
        ASSERT_EQ(innerOp, Zaban::AST::Expressions::BinaryOperator::Mul);
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
        EXPECT_EQ(h.text(first->get_name()), "a");
        auto second = fn->get_arg_at(1);
        ASSERT_NE(second, nullptr);
        EXPECT_EQ(h.text(second->get_name()), "b");
        auto second_type =
            std::dynamic_pointer_cast<Primitive>(second->get_annotation());
        ASSERT_NE(second_type, nullptr);
        ASSERT_EQ(second_type->get_specifierc(), 1u);
        EXPECT_EQ(second_type->get_specifier_at(0).kind, CTokenKind::Int);

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

    TEST(CParserTest, IfStmt) {
        Harness h{"int add(int a) {if (a>2) {return 2;} else {return 3;}}"};

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
        auto retType = fn->get_return_type();
        ASSERT_NE(retType, nullptr);
        auto intType =
            retType->dyn_cast<Langs::CLang::AST::Annotations::
                                  PrimitiveAnnotationNode<COffsetType>>();
        ASSERT_NE(intType, nullptr);
        ASSERT_EQ(intType->get_specifier_at(0).kind, CTokenKind::Int);

        auto body = fn->get_body();
        ASSERT_NE(body, nullptr);
        auto block = body->dyn_cast<
            Zaban::AST::Statements::BlockStatement<COffsetType>>();
        ASSERT_NE(block, nullptr);
        auto stmt = block->get_statement(0);
        ASSERT_NE(stmt, nullptr);

        auto ifstmt = stmt->dyn_cast<Zaban::Langs::CLang::AST::Statements::
                                         IfStatementNode<COffsetType>>();
        ASSERT_NE(ifstmt, nullptr);
        auto condition = ifstmt->get_condition();
        ASSERT_NE(condition, nullptr);
        auto conditionExpr = condition->dyn_cast<
            Zaban::AST::Expressions::BinaryExpression<COffsetType>>();
        ASSERT_NE(conditionExpr, nullptr);
        ASSERT_EQ(conditionExpr->get_op(),
                  Zaban::AST::Expressions::BinaryOperator::Gt);

        auto ifBody = ifstmt->get_then();
        ASSERT_NE(ifBody, nullptr);
        auto thenBlock = ifBody->dyn_cast<
            Zaban::AST::Statements::BlockStatement<COffsetType>>();
        ASSERT_NE(thenBlock, nullptr);
        ASSERT_EQ(thenBlock->statementc(), 1u);
        auto thenStmt = thenBlock->get_statement(0);
        ASSERT_NE(thenStmt, nullptr);

        auto ret = thenStmt->dyn_cast<
            Zaban::AST::Statements::FlowStatement<COffsetType>>();
        ASSERT_NE(ret, nullptr);
        EXPECT_EQ(ret->get_type(), Zaban::AST::Statements::FlowKind::Return);

        auto elseBody = ifstmt->get_else();
        ASSERT_NE(elseBody, nullptr);
        auto elseBlock = elseBody->dyn_cast<
            Zaban::AST::Statements::BlockStatement<COffsetType>>();
        ASSERT_NE(elseBlock, nullptr);
        ASSERT_EQ(elseBlock->statementc(), 1);
        auto elseStmt = elseBlock->get_statement(0);
        ASSERT_NE(elseStmt, nullptr);

        auto elseRet = elseStmt->dyn_cast<
            Zaban::AST::Statements::FlowStatement<COffsetType>>();
        ASSERT_NE(elseRet, nullptr);
        EXPECT_EQ(elseRet->get_type(),
                  Zaban::AST::Statements::FlowKind::Return);
    }

    TEST(CParserTest, EmptyTranslationUnits) {
        for (auto src: {"", " \n\t", "// comment", "/* comment */"}) {
            SCOPED_TRACE(src);
            Harness h{src};
            ASSERT_TRUE(h.upstream_clean());
            EXPECT_TRUE(h.parser_clean());
            EXPECT_TRUE(h.module_.empty());
        }
    }

    TEST(CParserTest, WhileAndDoWhileStructure) {
        Harness h{"void f() { while (x < 3) x++; do { x--; } while (x > 0); }"};
        ASSERT_TRUE(h.upstream_clean());
        ASSERT_TRUE(h.parser_clean());
        auto block = h.body();
        ASSERT_NE(block, nullptr);
        ASSERT_EQ(block->statementc(), 2u);
        auto pre = std::dynamic_pointer_cast<While>(block->get_statement(0));
        ASSERT_NE(pre, nullptr);
        EXPECT_EQ(h.text(pre->get_condition()), "x < 3");
        auto body = std::dynamic_pointer_cast<
            Statements::ExpressionStatement<COffsetType>>(pre->get_body());
        ASSERT_NE(body, nullptr);
        auto inc = std::dynamic_pointer_cast<Suffix>(body->get());
        ASSERT_NE(inc, nullptr);
        EXPECT_EQ(inc->get_operator(), Expressions::SuffixOperator::AddAdd);
        EXPECT_EQ(h.text(inc->get_expr()), "x");
        auto post = std::dynamic_pointer_cast<DoWhile>(block->get_statement(1));
        ASSERT_NE(post, nullptr);
        EXPECT_EQ(h.text(post->get_condition()), "x > 0");
        auto post_body = std::dynamic_pointer_cast<Block>(post->get_body());
        ASSERT_NE(post_body, nullptr);
        ASSERT_EQ(post_body->statementc(), 1u);
        EXPECT_EQ(h.text(post_body->get_statement(0)), "x--;");
        EXPECT_EQ(h.text(post), "do { x--; } while (x > 0);");
    }

    TEST(CParserTest, ForDeclarationInitializer) {
        Harness h{"void f() { for (int i = 0; i < 3; i++) { continue; } }"};
        ASSERT_TRUE(h.upstream_clean());
        ASSERT_TRUE(h.parser_clean());
        auto block = h.body();
        ASSERT_NE(block, nullptr);
        ASSERT_EQ(block->statementc(), 1u);
        auto loop = std::dynamic_pointer_cast<For>(block->get_statement(0));
        ASSERT_NE(loop, nullptr);
        auto init = std::dynamic_pointer_cast<
            Statements::DeclarationStatement<COffsetType>>(loop->get_init());
        ASSERT_NE(init, nullptr);
        auto decl = std::dynamic_pointer_cast<Let>(init->get());
        ASSERT_NE(decl, nullptr);
        EXPECT_EQ(h.text(decl->get_identifier()), "i");
        EXPECT_EQ(h.text(decl->get_initializer()), "0");
        EXPECT_EQ(h.text(loop->get_condition()), "i < 3");
        auto step = std::dynamic_pointer_cast<Suffix>(loop->get_step());
        ASSERT_NE(step, nullptr);
        EXPECT_EQ(step->get_operator(), Expressions::SuffixOperator::AddAdd);
        EXPECT_EQ(h.text(step->get_expr()), "i");
        auto body = std::dynamic_pointer_cast<Block>(loop->get_body());
        ASSERT_NE(body, nullptr);
        ASSERT_EQ(body->statementc(), 1u);
        auto flow = std::dynamic_pointer_cast<Flow>(body->get_statement(0));
        ASSERT_NE(flow, nullptr);
        EXPECT_EQ(flow->get_type(), Statements::FlowKind::Continue);
    }

    TEST(CParserTest, ForOptionalClauses) {
        // Cover every combination, including an expression initializer.
        for (unsigned mask = 0; mask < 8; ++mask) {
            SCOPED_TRACE(mask);
            const std::string init      = (mask & 1) ? "i = 0" : "";
            const std::string condition = (mask & 2) ? "i < 3" : "";
            const std::string step      = (mask & 4) ? "i++" : "";
            Harness h{"void f() { for (" + init + ";" + condition + ";" + step +
                      ") ; }"};
            ASSERT_TRUE(h.upstream_clean());
            ASSERT_TRUE(h.parser_clean());
            auto block = h.body();
            ASSERT_NE(block, nullptr);
            ASSERT_EQ(block->statementc(), 1u);
            auto loop = std::dynamic_pointer_cast<For>(block->get_statement(0));
            ASSERT_NE(loop, nullptr);
            EXPECT_EQ(loop->get_init() != nullptr, (mask & 1) != 0);
            EXPECT_EQ(loop->get_condition() != nullptr, (mask & 2) != 0);
            EXPECT_EQ(loop->get_step() != nullptr, (mask & 4) != 0);
            if (mask & 1) {
                auto stmt = std::dynamic_pointer_cast<
                    Statements::ExpressionStatement<COffsetType>>(
                    loop->get_init());
                ASSERT_NE(stmt, nullptr);
                auto assignment =
                    std::dynamic_pointer_cast<Assignment>(stmt->get());
                ASSERT_NE(assignment, nullptr);
                EXPECT_EQ(assignment->get_operation(),
                          Expressions::AssignmentOperator::None);
                EXPECT_EQ(h.text(assignment->get_left()), "i");
            }
            if (mask & 2) EXPECT_EQ(h.text(loop->get_condition()), condition);
            if (mask & 4) EXPECT_EQ(h.text(loop->get_step()), step);
            EXPECT_EQ(h.text(loop->get_body()), ";");
        }
    }

    TEST(CParserTest, NestedLoopControlAndDepthRestoration) {
        Harness h{
            "void f() { while (a) { for (;;) { continue; break; } continue; "
            "break; } break; continue; }"};
        ASSERT_TRUE(h.upstream_clean());
        auto diags = h.get_diags();
        ASSERT_EQ(diags.size(), 2u);
        EXPECT_EQ(diags[0].kind(),
                  CParserDiagnosticKind::ErrorBreakOutsideLoop);
        EXPECT_EQ(diags[1].kind(),
                  CParserDiagnosticKind::ErrorContinueOutsideLoop);
        EXPECT_EQ(diags[0].range().begin, h.source_.rfind("break"));
        EXPECT_EQ(diags[1].range().begin, h.source_.rfind("continue"));
        auto block = h.body();
        ASSERT_NE(block, nullptr);
        EXPECT_EQ(block->statementc(), 3u);
    }

    TEST(CParserTest, FailedLoopBodyRestoresDepth) {
        for (auto loop: {"while (1)", "for (;;)", "do"}) {
            SCOPED_TRACE(loop);
            Harness h{std::string("void f() { ") + loop +
                      " +; break; continue; return; }"};
            ASSERT_TRUE(h.upstream_clean());
            auto diags = h.get_diags();
            ASSERT_EQ(diags.size(), 3u);
            EXPECT_EQ(diags[0].kind(),
                      CParserDiagnosticKind::ErrorExpectedExpression);
            EXPECT_EQ(diags[1].kind(),
                      CParserDiagnosticKind::ErrorBreakOutsideLoop);
            EXPECT_EQ(diags[2].kind(),
                      CParserDiagnosticKind::ErrorContinueOutsideLoop);
            auto block = h.body();
            ASSERT_NE(block, nullptr);
            ASSERT_EQ(block->statementc(), 3u);
            auto ret = std::dynamic_pointer_cast<Flow>(block->get_statement(2));
            ASSERT_NE(ret, nullptr);
            EXPECT_EQ(ret->get_type(), Statements::FlowKind::Return);
            EXPECT_EQ(ret->get_expr(), nullptr);
        }
    }

    TEST(CParserTest, RecoveryPreservesFollowingGlobal) {
        Harness h{"int x = ; int y; int f() { return 2; }"};
        ASSERT_TRUE(h.upstream_clean());
        auto diags = h.get_diags();
        ASSERT_EQ(diags.size(), 1u);
        EXPECT_EQ(diags[0].kind(),
                  CParserDiagnosticKind::ErrorExpectedExpression);
        ASSERT_EQ(h.module_.size(), 2u);
        auto decl = h.declaration();
        ASSERT_NE(decl, nullptr);
        EXPECT_EQ(h.text(decl->get_identifier()), "y");
        ASSERT_NE(h.function(1), nullptr);
    }

    TEST(CParserTest, RecoveryPreservesFollowingStatement) {
        Harness h{"int f() { int x = ; return 2; }"};
        ASSERT_TRUE(h.upstream_clean());
        auto diags = h.get_diags();
        ASSERT_EQ(diags.size(), 1u);
        EXPECT_EQ(diags[0].kind(),
                  CParserDiagnosticKind::ErrorExpectedExpression);
        auto block = h.body();
        ASSERT_NE(block, nullptr);
        ASSERT_EQ(block->statementc(), 1u);
        EXPECT_EQ(h.text(h.returned()), "2");
    }

    TEST(CParserTest, RecoverySkipsMalformedFunctionBody) {
        Harness h{"int bad( { int local; { int nested; } } int good;"};
        ASSERT_TRUE(h.upstream_clean());
        EXPECT_FALSE(h.parser_clean());
        ASSERT_EQ(h.module_.size(), 1u);
        auto decl = h.declaration();
        ASSERT_NE(decl, nullptr);
        EXPECT_EQ(h.text(decl->get_identifier()), "good");
    }

    TEST(CParserTest, AssignmentIsRightAssociative) {
        Harness h{"int f() { return a = b = 4; }"};
        ASSERT_TRUE(h.upstream_clean());
        ASSERT_TRUE(h.parser_clean());
        auto outer = std::dynamic_pointer_cast<Assignment>(h.returned());
        ASSERT_NE(outer, nullptr);
        EXPECT_EQ(outer->get_operation(),
                  Expressions::AssignmentOperator::None);
        EXPECT_EQ(h.text(outer->get_left()), "a");
        auto rhs = outer->get_right();
        ASSERT_TRUE(std::holds_alternative<CExpression>(rhs));
        auto inner =
            std::dynamic_pointer_cast<Assignment>(std::get<CExpression>(rhs));
        ASSERT_NE(inner, nullptr);
        EXPECT_EQ(inner->get_operation(),
                  Expressions::AssignmentOperator::None);
        EXPECT_EQ(h.text(inner->get_left()), "b");
        auto value = inner->get_right();
        ASSERT_TRUE(std::holds_alternative<CExpression>(value));
        EXPECT_EQ(h.text(std::get<CExpression>(value)), "4");
    }

    TEST(CParserTest, EveryCompoundAssignmentOperator) {
        using Op = Expressions::AssignmentOperator;
        const std::pair<std::string_view, Op> cases[] = {
            {"+=", Op::Add},  {"-=", Op::Sub}, {"*=", Op::Mul}, {"/=", Op::Div},
            {"%=", Op::Mod},  {"&=", Op::And}, {"|=", Op::Or},  {"^=", Op::Xor},
            {"<<=", Op::Shl}, {">>=", Op::Shr}};
        for (auto [token, expected]: cases) {
            SCOPED_TRACE(token);
            Harness h{"int f() { return x " + std::string(token) + " 2; }"};
            ASSERT_TRUE(h.upstream_clean());
            ASSERT_TRUE(h.parser_clean());
            auto expr = std::dynamic_pointer_cast<Assignment>(h.returned());
            ASSERT_NE(expr, nullptr);
            EXPECT_EQ(expr->get_operation(), expected);
            EXPECT_EQ(h.text(expr->get_left()), "x");
            auto rhs = expr->get_right();
            ASSERT_TRUE(std::holds_alternative<CExpression>(rhs));
            EXPECT_EQ(h.text(std::get<CExpression>(rhs)), "2");
        }
    }

    TEST(CParserTest, BinaryPrecedenceBoundaries) {
        using Op = Expressions::BinaryOperator;
        struct Case {
            const char* src;
            Op          outer;
            Op          inner;
        };
        const Case cases[] = {{"a || b && c", Op::Lor, Op::Land},
                              {"a && b | c", Op::Land, Op::Or},
                              {"a | b ^ c", Op::Or, Op::Xor},
                              {"a ^ b & c", Op::Xor, Op::And},
                              {"a & b == c", Op::And, Op::Eq},
                              {"a == b < c", Op::Eq, Op::Lt},
                              {"a < b << c", Op::Lt, Op::Shl},
                              {"a << b + c", Op::Shl, Op::Add},
                              {"a + b * c", Op::Add, Op::Mul}};
        for (auto test: cases) {
            SCOPED_TRACE(test.src);
            Harness h{std::string("int f() { return ") + test.src + "; }"};
            ASSERT_TRUE(h.upstream_clean());
            ASSERT_TRUE(h.parser_clean());
            auto outer = std::dynamic_pointer_cast<Binary>(h.returned());
            ASSERT_NE(outer, nullptr);
            EXPECT_EQ(outer->get_op(), test.outer);
            EXPECT_EQ(h.text(outer->get_left()), "a");
            auto inner = std::dynamic_pointer_cast<Binary>(outer->get_right());
            ASSERT_NE(inner, nullptr);
            EXPECT_EQ(inner->get_op(), test.inner);
            EXPECT_EQ(h.text(inner->get_left()), "b");
            EXPECT_EQ(h.text(inner->get_right()), "c");
        }
    }

    TEST(CParserTest, GroupOverridesPrecedence) {
        Harness h{"int f() { return (a + b) * c; }"};
        ASSERT_TRUE(h.upstream_clean());
        ASSERT_TRUE(h.parser_clean());
        auto mul = std::dynamic_pointer_cast<Binary>(h.returned());
        ASSERT_NE(mul, nullptr);
        EXPECT_EQ(mul->get_op(), Expressions::BinaryOperator::Mul);
        auto group = std::dynamic_pointer_cast<Group>(mul->get_left());
        ASSERT_NE(group, nullptr);
        EXPECT_EQ(h.text(group), "(a + b)");
        auto add = std::dynamic_pointer_cast<Binary>(group->inner());
        ASSERT_NE(add, nullptr);
        EXPECT_EQ(add->get_op(), Expressions::BinaryOperator::Add);
        EXPECT_EQ(h.text(mul->get_right()), "c");
    }

    TEST(CParserTest, TernaryIsRightAssociative) {
        Harness h{"int f() { return a ? b : c ? d : e; }"};
        ASSERT_TRUE(h.upstream_clean());
        ASSERT_TRUE(h.parser_clean());
        auto outer = std::dynamic_pointer_cast<Ternary>(h.returned());
        ASSERT_NE(outer, nullptr);
        EXPECT_EQ(h.text(outer->get_condition()), "a");
        EXPECT_EQ(h.text(outer->get_true()), "b");
        auto inner = std::dynamic_pointer_cast<Ternary>(outer->get_false());
        ASSERT_NE(inner, nullptr);
        EXPECT_EQ(h.text(inner->get_condition()), "c");
        EXPECT_EQ(h.text(inner->get_true()), "d");
        EXPECT_EQ(h.text(inner->get_false()), "e");
    }

    TEST(CParserTest, TernaryAllowsAssignmentInTrueBranch) {
        Harness h{"int f() { return a || b ? c = d : e; }"};
        ASSERT_TRUE(h.upstream_clean());
        ASSERT_TRUE(h.parser_clean());
        auto expr = std::dynamic_pointer_cast<Ternary>(h.returned());
        ASSERT_NE(expr, nullptr);
        auto condition =
            std::dynamic_pointer_cast<Binary>(expr->get_condition());
        ASSERT_NE(condition, nullptr);
        EXPECT_EQ(condition->get_op(), Expressions::BinaryOperator::Lor);
        EXPECT_NE(std::dynamic_pointer_cast<Assignment>(expr->get_true()),
                  nullptr);
        EXPECT_EQ(h.text(expr->get_false()), "e");
    }

    TEST(CParserTest, PrefixOperatorsAndNesting) {
        using Op = Expressions::PrefixOperator;
        const std::pair<std::string_view, Op> cases[] = {
            {"++", Op::AddAdd}, {"--", Op::SubSub}, {"-", Op::Neg},
            {"+", Op::Pos},     {"!", Op::LNeg},    {"~", Op::BNeg},
            {"*", Op::Deref},   {"&", Op::AddrOf}};
        for (auto [token, expected]: cases) {
            SCOPED_TRACE(token);
            Harness h{"int f() { return " + std::string(token) + "x; }"};
            ASSERT_TRUE(h.upstream_clean());
            ASSERT_TRUE(h.parser_clean());
            auto expr = std::dynamic_pointer_cast<Prefix>(h.returned());
            ASSERT_NE(expr, nullptr);
            EXPECT_EQ(expr->get_operator(), expected);
            EXPECT_EQ(h.text(expr->get_expr()), "x");
        }
        Harness h{"int f() { return *&x; }"};
        ASSERT_TRUE(h.upstream_clean());
        ASSERT_TRUE(h.parser_clean());
        auto outer = std::dynamic_pointer_cast<Prefix>(h.returned());
        ASSERT_NE(outer, nullptr);
        EXPECT_EQ(outer->get_operator(), Op::Deref);
        auto inner = std::dynamic_pointer_cast<Prefix>(outer->get_expr());
        ASSERT_NE(inner, nullptr);
        EXPECT_EQ(inner->get_operator(), Op::AddrOf);
        EXPECT_EQ(h.text(inner->get_expr()), "x");
    }

    TEST(CParserTest, CallsAndPostfixChain) {
        Harness h{"int f() { g(); g(a, b); return g(a)[i]++; }"};
        ASSERT_TRUE(h.upstream_clean());
        ASSERT_TRUE(h.parser_clean());
        auto block = h.body();
        ASSERT_NE(block, nullptr);
        ASSERT_EQ(block->statementc(), 3u);
        for (std::size_t i = 0; i < 2; ++i) {
            auto stmt = std::dynamic_pointer_cast<
                Statements::ExpressionStatement<COffsetType>>(
                block->get_statement(i));
            ASSERT_NE(stmt, nullptr);
            auto call = std::dynamic_pointer_cast<Call>(stmt->get());
            ASSERT_NE(call, nullptr);
            EXPECT_EQ(h.text(call->get_callee()), "g");
            ASSERT_EQ(call->get_argc(), i == 0 ? 0u : 2u);
            if (i == 1) {
                EXPECT_EQ(h.text(call->get_arg_at(0)), "a");
                EXPECT_EQ(h.text(call->get_arg_at(1)), "b");
            }
        }
        auto suffix = std::dynamic_pointer_cast<Suffix>(h.returned(2));
        ASSERT_NE(suffix, nullptr);
        EXPECT_EQ(suffix->get_operator(), Expressions::SuffixOperator::AddAdd);
        auto index = std::dynamic_pointer_cast<Index>(suffix->get_expr());
        ASSERT_NE(index, nullptr);
        EXPECT_EQ(h.text(index->access()), "i");
        auto call = std::dynamic_pointer_cast<Call>(index->expr());
        ASSERT_NE(call, nullptr);
        ASSERT_EQ(call->get_argc(), 1u);
        EXPECT_EQ(h.text(call->get_callee()), "g");
        EXPECT_EQ(h.text(call->get_arg_at(0)), "a");
    }

    TEST(CParserTest, MemberAccessDistinguishesDotAndArrow) {
        Harness h{"int f() { return p->next.value--; }"};
        ASSERT_TRUE(h.upstream_clean());
        ASSERT_TRUE(h.parser_clean());
        auto suffix = std::dynamic_pointer_cast<Suffix>(h.returned());
        ASSERT_NE(suffix, nullptr);
        EXPECT_EQ(suffix->get_operator(), Expressions::SuffixOperator::SubSub);
        auto dot = std::dynamic_pointer_cast<Member>(suffix->get_expr());
        ASSERT_NE(dot, nullptr);
        EXPECT_FALSE(dot->is_arrow());
        EXPECT_EQ(h.text(dot->get_member()), "value");
        auto arrow = std::dynamic_pointer_cast<Member>(dot->get_base());
        ASSERT_NE(arrow, nullptr);
        EXPECT_TRUE(arrow->is_arrow());
        EXPECT_EQ(h.text(arrow->get_base()), "p");
        EXPECT_EQ(h.text(arrow->get_member()), "next");
    }

    TEST(CParserTest, PrimitiveSpecifiersAndPointerDepth) {
        Harness h{"unsigned long int x; int **p;"};
        ASSERT_TRUE(h.upstream_clean());
        ASSERT_TRUE(h.parser_clean());
        ASSERT_EQ(h.module_.size(), 2u);
        auto x = h.declaration();
        ASSERT_NE(x, nullptr);
        auto type = std::dynamic_pointer_cast<Primitive>(x->get_annotation());
        ASSERT_NE(type, nullptr);
        ASSERT_EQ(type->get_specifierc(), 3u);
        EXPECT_EQ(type->get_specifier_at(0).kind, CTokenKind::Unsigned);
        EXPECT_EQ(type->get_specifier_at(1).kind, CTokenKind::Long);
        EXPECT_EQ(type->get_specifier_at(2).kind, CTokenKind::Int);
        auto p = h.declaration(1);
        ASSERT_NE(p, nullptr);
        auto outer = std::dynamic_pointer_cast<Pointer>(p->get_annotation());
        ASSERT_NE(outer, nullptr);
        auto inner = std::dynamic_pointer_cast<Pointer>(outer->get_pointee());
        ASSERT_NE(inner, nullptr);
        auto base = std::dynamic_pointer_cast<Primitive>(inner->get_pointee());
        ASSERT_NE(base, nullptr);
        ASSERT_EQ(base->get_specifierc(), 1u);
        EXPECT_EQ(base->get_specifier_at(0).kind, CTokenKind::Int);
    }

    TEST(CParserTest, MultidimensionalArrayAndNestedInitializer) {
        Harness h{"int a[2][3] = {{1,2,3}, {4,5,6},};"};
        ASSERT_TRUE(h.upstream_clean());
        ASSERT_TRUE(h.parser_clean());
        auto decl = h.declaration();
        ASSERT_NE(decl, nullptr);
        auto outer = std::dynamic_pointer_cast<Array>(decl->get_annotation());
        ASSERT_NE(outer, nullptr);
        EXPECT_EQ(h.text(outer->get_num_elements()), "2");
        auto inner = std::dynamic_pointer_cast<Array>(outer->get_element());
        ASSERT_NE(inner, nullptr);
        EXPECT_EQ(h.text(inner->get_num_elements()), "3");
        EXPECT_NE(std::dynamic_pointer_cast<Primitive>(inner->get_element()),
                  nullptr);
        auto list =
            std::dynamic_pointer_cast<InitializerList>(decl->get_initializer());
        ASSERT_NE(list, nullptr);
        ASSERT_EQ(list->get_elementc(), 2u);
        for (std::size_t row = 0; row < 2; ++row) {
            auto nested = std::dynamic_pointer_cast<InitializerList>(
                list->get_element_at(row));
            ASSERT_NE(nested, nullptr);
            ASSERT_EQ(nested->get_elementc(), 3u);
            for (std::size_t col = 0; col < 3; ++col) {
                EXPECT_EQ(h.text(nested->get_element_at(col)),
                          std::to_string(row * 3 + col + 1));
            }
        }
    }

    TEST(CParserTest, UnsizedArrayAndTrailingInitializerComma) {
        Harness h{"int a[] = {1,2,};"};
        ASSERT_TRUE(h.upstream_clean());
        ASSERT_TRUE(h.parser_clean());
        auto decl = h.declaration();
        ASSERT_NE(decl, nullptr);
        auto array = std::dynamic_pointer_cast<Array>(decl->get_annotation());
        ASSERT_NE(array, nullptr);
        EXPECT_EQ(array->get_num_elements(), nullptr);
        auto list =
            std::dynamic_pointer_cast<InitializerList>(decl->get_initializer());
        ASSERT_NE(list, nullptr);
        ASSERT_EQ(list->get_elementc(), 2u);
        EXPECT_EQ(h.text(list->get_element_at(0)), "1");
        EXPECT_EQ(h.text(list->get_element_at(1)), "2");
    }

    TEST(CParserTest, FunctionPrototypesAndVariadicParameters) {
        Harness h{
            "int f(void); int g(int, char*); int v(int x, ...); int e(); int "
            "p(void *ptr, int a[]);"};
        ASSERT_TRUE(h.upstream_clean());
        ASSERT_TRUE(h.parser_clean());
        ASSERT_EQ(h.module_.size(), 5u);
        const std::size_t counts[] = {0, 2, 2, 0, 2};
        for (std::size_t i = 0; i < 5; ++i) {
            auto fn = h.function(i);
            ASSERT_NE(fn, nullptr);
            EXPECT_EQ(fn->get_body(), nullptr);
            ASSERT_EQ(fn->get_argc(), counts[i]);
            for (std::size_t arg = 0; arg < counts[i]; ++arg) {
                ASSERT_NE(fn->get_arg_at(arg), nullptr);
            }
        }
        auto g = h.function(1);
        ASSERT_NE(g, nullptr);
        ASSERT_EQ(g->get_argc(), 2u);
        EXPECT_EQ(g->get_arg_at(0)->get_name(), nullptr);
        EXPECT_EQ(g->get_arg_at(1)->get_name(), nullptr);
        EXPECT_NE(std::dynamic_pointer_cast<Pointer>(
                      g->get_arg_at(1)->get_annotation()),
                  nullptr);
        auto v = h.function(2);
        ASSERT_NE(v, nullptr);
        ASSERT_EQ(v->get_argc(), 2u);
        EXPECT_FALSE(v->get_arg_at(0)->is_vararg());
        EXPECT_TRUE(v->get_arg_at(1)->is_vararg());
        EXPECT_EQ(v->get_arg_at(1)->get_name(), nullptr);
        EXPECT_EQ(h.text(v->get_arg_at(1)), "...");
        auto p = h.function(4);
        ASSERT_NE(p, nullptr);
        ASSERT_EQ(p->get_argc(), 2u);
        EXPECT_EQ(h.text(p->get_arg_at(0)->get_name()), "ptr");
        EXPECT_NE(std::dynamic_pointer_cast<Pointer>(
                      p->get_arg_at(0)->get_annotation()),
                  nullptr);
        auto array = std::dynamic_pointer_cast<Array>(
            p->get_arg_at(1)->get_annotation());
        ASSERT_NE(array, nullptr);
        EXPECT_EQ(array->get_num_elements(), nullptr);
    }

    TEST(CParserTest, DanglingElseBindsToNearestIf) {
        Harness h{"int f() { if (a) if (b) return 1; else return 2; }"};
        ASSERT_TRUE(h.upstream_clean());
        ASSERT_TRUE(h.parser_clean());
        auto block = h.body();
        ASSERT_NE(block, nullptr);
        ASSERT_EQ(block->statementc(), 1u);
        auto outer = std::dynamic_pointer_cast<If>(block->get_statement(0));
        ASSERT_NE(outer, nullptr);
        EXPECT_EQ(outer->get_else(), nullptr);
        auto inner = std::dynamic_pointer_cast<If>(outer->get_then());
        ASSERT_NE(inner, nullptr);
        EXPECT_EQ(h.text(inner->get_condition()), "b");
        EXPECT_EQ(h.text(inner->get_then()), "return 1;");
        EXPECT_EQ(h.text(inner->get_else()), "return 2;");
    }

    TEST(CParserTest, ElseIfChain) {
        Harness h{
            "int f() { if (a) return 1; else if (b) return 2; else return 3; "
            "}"};
        ASSERT_TRUE(h.upstream_clean());
        ASSERT_TRUE(h.parser_clean());
        auto block = h.body();
        ASSERT_NE(block, nullptr);
        ASSERT_EQ(block->statementc(), 1u);
        auto outer = std::dynamic_pointer_cast<If>(block->get_statement(0));
        ASSERT_NE(outer, nullptr);
        auto inner = std::dynamic_pointer_cast<If>(outer->get_else());
        ASSERT_NE(inner, nullptr);
        EXPECT_EQ(h.text(inner->get_condition()), "b");
        EXPECT_EQ(h.text(inner->get_else()), "return 3;");
    }

    TEST(CParserTest, RejectsUnbracedDeclarations) {
        for (auto stmt:
             {"if (a) int x;", "if (a) ; else int x;", "while (a) int x;",
              "do int x; while (a);", "for (;;) int x;"}) {
            SCOPED_TRACE(stmt);
            Harness h{std::string("void f() { ") + stmt + " }"};
            ASSERT_TRUE(h.upstream_clean());
            auto diags = h.get_diags();
            ASSERT_EQ(diags.size(), 1u);
            EXPECT_EQ(diags[0].kind(),
                      CParserDiagnosticKind::ErrorDeclarationNotAllowed);
            EXPECT_EQ(std::string_view(h.source_).substr(
                          diags[0].range().begin,
                          diags[0].range().end - diags[0].range().begin),
                      "int x;");
        }
    }

    TEST(CParserTest, RejectsNestedFunctionDefinitions) {
        Harness h{"void outer() { int inner() {} } int good;"};
        ASSERT_TRUE(h.upstream_clean());
        EXPECT_FALSE(h.parser_clean());
        auto diags = h.get_diags();
        ASSERT_FALSE(diags.empty());
        EXPECT_EQ(diags[0].kind(),
                  CParserDiagnosticKind::ErrorDeclarationNotAllowed);
        auto last = h.declaration(h.module_.size() - 1);
        ASSERT_NE(last, nullptr);
        EXPECT_EQ(h.text(last->get_identifier()), "good");
    }

    TEST(CParserTest, MalformedExpressionsReportSpecificDiagnostics) {
        using Kind                                 = CParserDiagnosticKind;
        const std::pair<const char*, Kind> cases[] = {
            {"a ? b c", Kind::ErrorExpectedToken},
            {"a ? : c", Kind::ErrorExpectedExpression},
            {"a ? b :", Kind::ErrorExpectedExpression},
            {"a +", Kind::ErrorExpectedExpression},
            {"g(a,)", Kind::ErrorExpectedExpression},
            {"a[]", Kind::ErrorExpectedExpression},
            {"a[1", Kind::ErrorExpectedToken},
            {"p->", Kind::ErrorExpectedIdentifier},
            {"p.", Kind::ErrorExpectedIdentifier}};
        for (auto [src, expected]: cases) {
            SCOPED_TRACE(src);
            Harness h{std::string("int f() { return ") + src + "; }"};
            ASSERT_TRUE(h.upstream_clean());
            auto diags = h.get_diags();
            ASSERT_EQ(diags.size(), 1u);
            EXPECT_EQ(diags[0].kind(), expected);
        }
    }

    TEST(CParserTest, InvalidParameterLists) {
        for (auto src:
             {"int f(...);", "int f(int x, ..., int y);", "int f(int x,);"}) {
            SCOPED_TRACE(src);
            Harness h{src};
            ASSERT_TRUE(h.upstream_clean());
            EXPECT_FALSE(h.parser_clean());
        }
    }

    TEST(CParserTest, TruncatedInputsTerminateWithDiagnostics) {
        for (auto src:
             {"int", "int f(", "int f() {", "int f() { return", "int a[",
              "int a[] = {1,", "int f() { while (", "int f() { for (;;)",
              "int f() { do", "int f() { return g("}) {
            SCOPED_TRACE(src);
            Harness h{src};
            ASSERT_TRUE(h.upstream_clean());
            EXPECT_FALSE(h.parser_clean());
            auto diags = h.get_diags();
            ASSERT_FALSE(diags.empty());
            for (const auto& diag: diags) {
                EXPECT_LE(diag.range().begin, diag.range().end);
                EXPECT_LE(diag.range().end, h.source_.size());
            }
        }
    }

    TEST(CParserTest, NodeAndDiagnosticSourceRanges) {
        Harness h{"  int f(int a) { return a + 2; }\n"};
        ASSERT_TRUE(h.upstream_clean());
        ASSERT_TRUE(h.parser_clean());
        auto decl = h.declaration();
        ASSERT_NE(decl, nullptr);
        EXPECT_EQ(decl->location().begin, 2u);
        EXPECT_EQ(decl->location().end, h.source_.size() - 1);
        EXPECT_EQ(h.text(decl), "int f(int a) { return a + 2; }");
        auto block = h.body();
        ASSERT_NE(block, nullptr);
        ASSERT_EQ(block->statementc(), 1u);
        EXPECT_EQ(h.text(block), "{ return a + 2; }");
        EXPECT_EQ(h.text(block->get_statement(0)), "return a + 2;");
        auto sum = std::dynamic_pointer_cast<Binary>(h.returned());
        ASSERT_NE(sum, nullptr);
        EXPECT_EQ(h.text(sum), "a + 2");
        EXPECT_EQ(h.text(sum->get_left()), "a");
        EXPECT_EQ(h.text(sum->get_right()), "2");

        Harness bad{"int f() { return 2 }"};
        ASSERT_TRUE(bad.upstream_clean());
        auto diags = bad.get_diags();
        ASSERT_EQ(diags.size(), 1u);
        EXPECT_EQ(diags[0].kind(), CParserDiagnosticKind::ErrorExpectedToken);
        EXPECT_EQ(diags[0].range().begin, bad.source_.find('}'));
        EXPECT_EQ(diags[0].range().end, bad.source_.find('}') + 1);
    }

    TEST(CParserTest, PreprocessorOutputFiltersDirectivesAndSkippedTokens) {
        Harness h{R"c(
#define VALUE 2
#if 0
this is not valid C !!!
#endif
int f() { return VALUE; }
)c"};
        ASSERT_TRUE(h.upstream_clean());
        ASSERT_TRUE(h.parser_clean());
        ASSERT_EQ(h.module_.size(), 1u);
        ASSERT_NE(h.returned(), nullptr);
        EXPECT_EQ(h.returned()->expr_kind(), ExpressionKind::Primary);
    }

    TEST(CParserTest, DeclaratorBindingAndMultipleDeclarations) {
        Harness h{"int *a[3], (*b)[3], *c = 0, d = 2;"};
        ASSERT_TRUE(h.upstream_clean());
        ASSERT_TRUE(h.parser_clean());
        ASSERT_EQ(h.module_.size(), 1u);
        auto group = std::dynamic_pointer_cast<DeclarationGroup>(h.module_[0]);
        ASSERT_NE(group, nullptr);
        EXPECT_EQ(group->kind(), DeclarationKind::GroupDecl);
        EXPECT_EQ(h.text(group), h.source_);
        const auto& decls = group->get_declarations();
        ASSERT_EQ(decls.size(), 4u);
        auto a = std::dynamic_pointer_cast<Let>(decls[0]);
        auto b = std::dynamic_pointer_cast<Let>(decls[1]);
        auto c = std::dynamic_pointer_cast<Let>(decls[2]);
        auto d = std::dynamic_pointer_cast<Let>(decls[3]);
        ASSERT_NE(a, nullptr);
        ASSERT_NE(b, nullptr);
        ASSERT_NE(c, nullptr);
        ASSERT_NE(d, nullptr);
        auto array = std::dynamic_pointer_cast<Array>(a->get_annotation());
        ASSERT_NE(array, nullptr);
        EXPECT_NE(std::dynamic_pointer_cast<Pointer>(array->get_element()),
                  nullptr);
        EXPECT_EQ(h.text(array->get_num_elements()), "3");
        auto pointer = std::dynamic_pointer_cast<Pointer>(b->get_annotation());
        ASSERT_NE(pointer, nullptr);
        EXPECT_NE(std::dynamic_pointer_cast<Array>(pointer->get_pointee()),
                  nullptr);
        EXPECT_NE(std::dynamic_pointer_cast<Pointer>(c->get_annotation()),
                  nullptr);
        EXPECT_NE(std::dynamic_pointer_cast<Primitive>(d->get_annotation()),
                  nullptr);
        EXPECT_EQ(h.text(c->get_initializer()), "0");
        EXPECT_EQ(h.text(d->get_initializer()), "2");
        EXPECT_EQ(h.text(b), "(*b)[3]");
    }

    TEST(CParserTest, FunctionDeclaratorsPreserveBindingAndBodies) {
        Harness h{
            "int *f(int); int (*fp)(int); int (*table[2])(void); "
            "int (*factory(void))(int) { return fp; }"};
        ASSERT_TRUE(h.upstream_clean());
        ASSERT_TRUE(h.parser_clean());
        ASSERT_EQ(h.module_.size(), 4u);
        auto f = h.function(0);
        ASSERT_NE(f, nullptr);
        EXPECT_NE(std::dynamic_pointer_cast<Pointer>(f->get_return_type()),
                  nullptr);
        auto fp = h.declaration(1);
        ASSERT_NE(fp, nullptr);
        EXPECT_EQ(fp->get_initializer(), nullptr);
        auto pointer = std::dynamic_pointer_cast<Pointer>(fp->get_annotation());
        ASSERT_NE(pointer, nullptr);
        auto signature =
            std::dynamic_pointer_cast<FunctionType>(pointer->get_pointee());
        ASSERT_NE(signature, nullptr);
        ASSERT_EQ(signature->get_parameters().size(), 1u);
        auto table = h.declaration(2);
        ASSERT_NE(table, nullptr);
        auto array = std::dynamic_pointer_cast<Array>(table->get_annotation());
        ASSERT_NE(array, nullptr);
        auto element = std::dynamic_pointer_cast<Pointer>(array->get_element());
        ASSERT_NE(element, nullptr);
        EXPECT_NE(
            std::dynamic_pointer_cast<FunctionType>(element->get_pointee()),
            nullptr);
        auto factory = h.function(3);
        ASSERT_NE(factory, nullptr);
        EXPECT_NE(factory->get_body(), nullptr);
        auto result =
            std::dynamic_pointer_cast<Pointer>(factory->get_return_type());
        ASSERT_NE(result, nullptr);
        EXPECT_NE(
            std::dynamic_pointer_cast<FunctionType>(result->get_pointee()),
            nullptr);
    }

    TEST(CParserTest, AbstractDeclaratorsAndC23EmptyParameterLists) {
        Harness h{
            "int f(int (*)(int), int (*)[3], int [2][3], int (int)); "
            "int empty(); int explicit_void(void);"};
        ASSERT_TRUE(h.upstream_clean());
        ASSERT_TRUE(h.parser_clean());
        auto f = h.function(0);
        ASSERT_NE(f, nullptr);
        ASSERT_EQ(f->get_argc(), 4u);
        for (std::size_t i = 0; i < 4; ++i)
            EXPECT_EQ(f->get_arg_at(i)->get_name(), nullptr);
        auto callback = std::dynamic_pointer_cast<Pointer>(
            f->get_arg_at(0)->get_annotation());
        ASSERT_NE(callback, nullptr);
        EXPECT_NE(
            std::dynamic_pointer_cast<FunctionType>(callback->get_pointee()),
            nullptr);
        auto row = std::dynamic_pointer_cast<Pointer>(
            f->get_arg_at(1)->get_annotation());
        ASSERT_NE(row, nullptr);
        EXPECT_NE(std::dynamic_pointer_cast<Array>(row->get_pointee()),
                  nullptr);
        auto matrix = std::dynamic_pointer_cast<Array>(
            f->get_arg_at(2)->get_annotation());
        ASSERT_NE(matrix, nullptr);
        EXPECT_EQ(h.text(matrix->get_num_elements()), "2");
        auto inner = std::dynamic_pointer_cast<Array>(matrix->get_element());
        ASSERT_NE(inner, nullptr);
        EXPECT_EQ(h.text(inner->get_num_elements()), "3");
        EXPECT_NE(std::dynamic_pointer_cast<FunctionType>(
                      f->get_arg_at(3)->get_annotation()),
                  nullptr);
        for (std::size_t i = 1; i <= 2; ++i) {
            auto decl = h.declaration(i);
            ASSERT_NE(decl, nullptr);
            auto type =
                std::dynamic_pointer_cast<FunctionType>(decl->get_annotation());
            ASSERT_NE(type, nullptr);
            EXPECT_TRUE(type->get_parameters().empty());
        }
    }

    TEST(CParserTest, DeclarationGroupsInMembersBlocksAndForInitializers) {
        Harness h{
            "struct S { int *p, n; int (*cb)(int); }; "
            "int f() { int a = 1, b = 2; for (int i = 0, j = 1; i < j; ++i) {} "
            "}"};
        ASSERT_TRUE(h.upstream_clean());
        ASSERT_TRUE(h.parser_clean());
        auto record = std::dynamic_pointer_cast<CStructDecl>(h.module_[0]);
        ASSERT_NE(record, nullptr);
        const auto& fields = record->get_annotation()->get_fields();
        ASSERT_EQ(fields.size(), 3u);
        EXPECT_NE(
            std::dynamic_pointer_cast<Pointer>(fields[0]->get_annotation()),
            nullptr);
        EXPECT_NE(
            std::dynamic_pointer_cast<Primitive>(fields[1]->get_annotation()),
            nullptr);
        auto block = h.body(1);
        ASSERT_NE(block, nullptr);
        ASSERT_EQ(block->statementc(), 2u);
        auto local = std::dynamic_pointer_cast<
            Statements::DeclarationStatement<COffsetType>>(
            block->get_statement(0));
        ASSERT_NE(local, nullptr);
        EXPECT_NE(std::dynamic_pointer_cast<DeclarationGroup>(local->get()),
                  nullptr);
        auto loop = std::dynamic_pointer_cast<For>(block->get_statement(1));
        ASSERT_NE(loop, nullptr);
        auto init = std::dynamic_pointer_cast<
            Statements::DeclarationStatement<COffsetType>>(loop->get_init());
        ASSERT_NE(init, nullptr);
        EXPECT_NE(std::dynamic_pointer_cast<DeclarationGroup>(init->get()),
                  nullptr);
    }

    TEST(CParserTest, MalformedDeclaratorsPreserveFollowingGlobals) {
        for (auto src: {"int (*p;", "int a,;", "int (*p)(int,);",
                        "int (*p)(int) {}", "int a, f() {}", "int f() = 1;"}) {
            SCOPED_TRACE(src);
            Harness h{std::string(src) + " int good;"};
            ASSERT_TRUE(h.upstream_clean());
            EXPECT_FALSE(h.parser_clean());
            ASSERT_FALSE(h.module_.empty());
            auto good = h.declaration(h.module_.size() - 1);
            ASSERT_NE(good, nullptr);
            EXPECT_EQ(h.text(good->get_identifier()), "good");
        }
    }

    TEST(CParserTest, EnumDefinitionsValuesAndRanges) {
        Harness h{
            "enum Color { RED, GREEN = 4, BLUE = GREEN + 1, "
            "LAST = 1 ? 8 : 9, }; enum Color color;"};
        ASSERT_TRUE(h.upstream_clean());
        ASSERT_TRUE(h.parser_clean());
        ASSERT_EQ(h.module_.size(), 2u);
        auto decl = std::dynamic_pointer_cast<CEnumDecl>(h.module_[0]);
        ASSERT_NE(decl, nullptr);
        EXPECT_EQ(decl->kind(), DeclarationKind::TypeDecl);
        EXPECT_EQ(h.text(decl),
                  "enum Color { RED, GREEN = 4, BLUE = GREEN + 1, "
                  "LAST = 1 ? 8 : 9, };");
        auto type = decl->get_annotation();
        ASSERT_NE(type, nullptr);
        EXPECT_EQ(type->get_base_kind(), BaseAnnotationKind::Enum);
        EXPECT_TRUE(type->has_definition());
        EXPECT_EQ(h.text(type->get_tag()), "Color");
        EXPECT_EQ(h.text(type),
                  "enum Color { RED, GREEN = 4, BLUE = GREEN + 1, "
                  "LAST = 1 ? 8 : 9, }");
        const auto& fields = type->get_fields();
        ASSERT_EQ(fields.size(), 4u);
        EXPECT_EQ(h.text(fields[0]->get_name()), "RED");
        EXPECT_EQ(fields[0]->get_initializer(), nullptr);
        EXPECT_EQ(fields[0]->get_annotation(), nullptr);
        EXPECT_FALSE(fields[0]->is_vararg());
        EXPECT_EQ(h.text(fields[1]), "GREEN = 4");
        EXPECT_EQ(h.text(fields[1]->get_initializer()), "4");
        EXPECT_EQ(h.text(fields[2]->get_initializer()), "GREEN + 1");
        EXPECT_NE(
            std::dynamic_pointer_cast<Binary>(fields[2]->get_initializer()),
            nullptr);
        EXPECT_NE(
            std::dynamic_pointer_cast<Ternary>(fields[3]->get_initializer()),
            nullptr);
        auto object = h.declaration(1);
        ASSERT_NE(object, nullptr);
        auto reference =
            std::dynamic_pointer_cast<CEnum>(object->get_annotation());
        ASSERT_NE(reference, nullptr);
        EXPECT_FALSE(reference->has_definition());
        EXPECT_TRUE(reference->get_fields().empty());
        EXPECT_EQ(h.text(reference->get_tag()), "Color");
    }

    TEST(CParserTest, EnumAnonymousDefinitionsTagsAndNestedMembers) {
        Harness h{
            "enum { FIRST, SECOND }; enum { ON, OFF } state; "
            "enum Tag; enum Tag *ptr; "
            "struct S { enum { SMALL, LARGE } size; };"};
        ASSERT_TRUE(h.upstream_clean());
        ASSERT_TRUE(h.parser_clean());
        ASSERT_EQ(h.module_.size(), 5u);
        auto anonymous = std::dynamic_pointer_cast<CEnumDecl>(h.module_[0]);
        ASSERT_NE(anonymous, nullptr);
        EXPECT_EQ(anonymous->get_annotation()->get_tag(), nullptr);
        EXPECT_TRUE(anonymous->get_annotation()->has_definition());
        auto object = h.declaration(1);
        ASSERT_NE(object, nullptr);
        auto object_type =
            std::dynamic_pointer_cast<CEnum>(object->get_annotation());
        ASSERT_NE(object_type, nullptr);
        EXPECT_EQ(object_type->get_tag(), nullptr);
        EXPECT_TRUE(object_type->has_definition());
        auto tag = std::dynamic_pointer_cast<CEnumDecl>(h.module_[2]);
        ASSERT_NE(tag, nullptr);
        EXPECT_FALSE(tag->get_annotation()->has_definition());
        auto ptr = h.declaration(3);
        ASSERT_NE(ptr, nullptr);
        auto pointer =
            std::dynamic_pointer_cast<Pointer>(ptr->get_annotation());
        ASSERT_NE(pointer, nullptr);
        EXPECT_NE(std::dynamic_pointer_cast<CEnum>(pointer->get_pointee()),
                  nullptr);
        auto record = std::dynamic_pointer_cast<CStructDecl>(h.module_[4]);
        ASSERT_NE(record, nullptr);
        const auto& members = record->get_annotation()->get_fields();
        ASSERT_EQ(members.size(), 1u);
        auto nested =
            std::dynamic_pointer_cast<CEnum>(members[0]->get_annotation());
        ASSERT_NE(nested, nullptr);
        EXPECT_EQ(nested->get_fields().size(), 2u);
    }

    TEST(CParserTest, EnumRecoveryPreservesEnumeratorsAndFollowingGlobals) {
        for (auto body:
             {"BAD = , KEPT", "BAD = {1, 2}, KEPT", "BAD = call(1, ), KEPT",
              "FIRST KEPT", ", KEPT", "BAD = 1 = 2, KEPT"}) {
            SCOPED_TRACE(body);
            Harness h{std::string("enum E { ") + body + " }; int good;"};
            ASSERT_TRUE(h.upstream_clean());
            EXPECT_FALSE(h.parser_clean());
            ASSERT_EQ(h.module_.size(), 2u);
            auto decl = std::dynamic_pointer_cast<CEnumDecl>(h.module_[0]);
            ASSERT_NE(decl, nullptr);
            const auto& fields = decl->get_annotation()->get_fields();
            ASSERT_FALSE(fields.empty());
            EXPECT_EQ(h.text(fields.back()->get_name()), "KEPT");
            auto good = h.declaration(1);
            ASSERT_NE(good, nullptr);
            EXPECT_EQ(h.text(good->get_identifier()), "good");
        }
    }

    TEST(CParserTest, EnumEmptyAndTruncatedBodiesReportErrors) {
        for (auto src: {"enum {}; int good;", "enum E { BAD = ( }; int good;",
                        "enum E { A; int good;"}) {
            SCOPED_TRACE(src);
            Harness h{src};
            ASSERT_TRUE(h.upstream_clean());
            EXPECT_FALSE(h.parser_clean());
            ASSERT_FALSE(h.module_.empty());
            auto good = h.declaration(h.module_.size() - 1);
            ASSERT_NE(good, nullptr);
            EXPECT_EQ(h.text(good->get_identifier()), "good");
        }
        for (auto src: {"enum", "enum;", "enum E *;", "enum E {",
                        "enum E { A =", "enum E { A,"}) {
            SCOPED_TRACE(src);
            Harness h{src};
            ASSERT_TRUE(h.upstream_clean());
            EXPECT_FALSE(h.parser_clean());
            EXPECT_TRUE(h.module_.empty());
        }
    }

    TEST(CParserTest, UnionDeclarationsReferencesAndRanges) {
        Harness h{
            "union Value; union Value { int i; float f; }; "
            "union Value *ptr;"};
        ASSERT_TRUE(h.upstream_clean());
        ASSERT_TRUE(h.parser_clean());
        ASSERT_EQ(h.module_.size(), 3u);
        auto forward = std::dynamic_pointer_cast<CUnionDecl>(h.module_[0]);
        ASSERT_NE(forward, nullptr);
        EXPECT_EQ(forward->kind(), DeclarationKind::TypeDecl);
        EXPECT_EQ(h.text(forward), "union Value;");
        auto reference = forward->get_annotation();
        ASSERT_NE(reference, nullptr);
        EXPECT_FALSE(reference->has_definition());
        EXPECT_TRUE(reference->get_variants().empty());
        EXPECT_EQ(h.text(reference->get_tag()), "Value");
        EXPECT_EQ(h.text(reference), "union Value");

        auto decl = std::dynamic_pointer_cast<CUnionDecl>(h.module_[1]);
        ASSERT_NE(decl, nullptr);
        EXPECT_EQ(h.text(decl), "union Value { int i; float f; };");
        auto type = decl->get_annotation();
        ASSERT_NE(type, nullptr);
        EXPECT_TRUE(type->has_definition());
        EXPECT_EQ(type->get_base_kind(), BaseAnnotationKind::Variant);
        EXPECT_EQ(h.text(type->get_tag()), "Value");
        EXPECT_EQ(h.text(type), "union Value { int i; float f; }");
        const auto& members = type->get_variants();
        ASSERT_EQ(members.size(), 2u);
        EXPECT_EQ(h.text(members[0]->get_name()), "i");
        EXPECT_EQ(h.text(members[0]), "int i;");
        EXPECT_EQ(h.text(members[1]->get_name()), "f");
        EXPECT_EQ(h.text(members[1]), "float f;");

        auto ptr = h.declaration(2);
        ASSERT_NE(ptr, nullptr);
        auto pointer =
            std::dynamic_pointer_cast<Pointer>(ptr->get_annotation());
        ASSERT_NE(pointer, nullptr);
        auto pointee =
            std::dynamic_pointer_cast<CUnion>(pointer->get_pointee());
        ASSERT_NE(pointee, nullptr);
        EXPECT_FALSE(pointee->has_definition());
        EXPECT_EQ(h.text(pointee->get_tag()), "Value");
    }

    TEST(CParserTest, UnionAnonymousObjectsAndMixedNesting) {
        Harness h{
            "struct Outer { union { int i; float f; } value; }; "
            "union { struct { int x; } point; int raw[2]; } object;"};
        ASSERT_TRUE(h.upstream_clean());
        ASSERT_TRUE(h.parser_clean());
        ASSERT_EQ(h.module_.size(), 2u);
        auto outer = std::dynamic_pointer_cast<CStructDecl>(h.module_[0]);
        ASSERT_NE(outer, nullptr);
        const auto& fields = outer->get_annotation()->get_fields();
        ASSERT_EQ(fields.size(), 1u);
        auto nested =
            std::dynamic_pointer_cast<CUnion>(fields[0]->get_annotation());
        ASSERT_NE(nested, nullptr);
        EXPECT_TRUE(nested->has_definition());
        EXPECT_EQ(nested->get_tag(), nullptr);
        EXPECT_EQ(nested->get_variants().size(), 2u);

        auto object = h.declaration(1);
        ASSERT_NE(object, nullptr);
        EXPECT_EQ(h.text(object->get_identifier()), "object");
        auto type = std::dynamic_pointer_cast<CUnion>(object->get_annotation());
        ASSERT_NE(type, nullptr);
        EXPECT_TRUE(type->has_definition());
        EXPECT_EQ(type->get_tag(), nullptr);
        const auto& members = type->get_variants();
        ASSERT_EQ(members.size(), 2u);
        auto point =
            std::dynamic_pointer_cast<CStruct>(members[0]->get_annotation());
        ASSERT_NE(point, nullptr);
        ASSERT_EQ(point->get_fields().size(), 1u);
        EXPECT_EQ(h.text(point->get_fields()[0]->get_name()), "x");
        auto raw =
            std::dynamic_pointer_cast<Array>(members[1]->get_annotation());
        ASSERT_NE(raw, nullptr);
        EXPECT_EQ(h.text(members[1]), "int raw[2];");
    }

    TEST(CParserTest, UnionMalformedMembersPreserveFollowingDeclarations) {
        for (auto member: {"int broken = {{1,2}};", "int broken[", ""}) {
            SCOPED_TRACE(member);
            Harness h{std::string("union U { ") + member + " }; int good;"};
            ASSERT_TRUE(h.upstream_clean());
            EXPECT_FALSE(h.parser_clean());
            ASSERT_EQ(h.module_.size(), 2u);
            auto decl = std::dynamic_pointer_cast<CUnionDecl>(h.module_[0]);
            ASSERT_NE(decl, nullptr);
            EXPECT_TRUE(decl->get_annotation()->get_variants().empty());
            auto good = h.declaration(1);
            ASSERT_NE(good, nullptr);
            EXPECT_EQ(h.text(good->get_identifier()), "good");
        }
    }

    TEST(CParserTest, UnionInvalidAndTruncatedDeclarationsReportErrors) {
        for (auto src: {"union;", "union U *;", "union { int x; };", "union",
                        "union U { int x;"}) {
            SCOPED_TRACE(src);
            Harness h{src};
            ASSERT_TRUE(h.upstream_clean());
            EXPECT_FALSE(h.parser_clean());
            EXPECT_TRUE(h.module_.empty());
        }
    }

    TEST(CParserTest, StructForwardDeclarationAndTagReference) {
        Harness h{"struct Point; struct Point p; struct Point *ptr;"};
        ASSERT_TRUE(h.upstream_clean());
        ASSERT_TRUE(h.parser_clean());
        ASSERT_EQ(h.module_.size(), 3u);
        auto forward = std::dynamic_pointer_cast<CStructDecl>(h.module_[0]);
        ASSERT_NE(forward, nullptr);
        EXPECT_EQ(forward->kind(), DeclarationKind::TypeDecl);
        EXPECT_EQ(h.text(forward), "struct Point;");
        auto reference = forward->get_annotation();
        ASSERT_NE(reference, nullptr);
        EXPECT_FALSE(reference->has_definition());
        EXPECT_TRUE(reference->get_fields().empty());
        EXPECT_EQ(h.text(reference->get_tag()), "Point");
        EXPECT_EQ(h.text(reference), "struct Point");
        auto p = h.declaration(1);
        ASSERT_NE(p, nullptr);
        EXPECT_EQ(h.text(p->get_identifier()), "p");
        auto type = std::dynamic_pointer_cast<CStruct>(p->get_annotation());
        ASSERT_NE(type, nullptr);
        EXPECT_FALSE(type->has_definition());
        EXPECT_EQ(h.text(type->get_tag()), "Point");
        auto ptr = h.declaration(2);
        ASSERT_NE(ptr, nullptr);
        auto pointer =
            std::dynamic_pointer_cast<Pointer>(ptr->get_annotation());
        ASSERT_NE(pointer, nullptr);
        auto pointee =
            std::dynamic_pointer_cast<CStruct>(pointer->get_pointee());
        ASSERT_NE(pointee, nullptr);
        EXPECT_FALSE(pointee->has_definition());
        EXPECT_EQ(h.text(pointee->get_tag()), "Point");
    }

    TEST(CParserTest, StructDefinitionFieldsAndRanges) {
        Harness h{"  struct Point { int x; unsigned long y; };\n"};
        ASSERT_TRUE(h.upstream_clean());
        ASSERT_TRUE(h.parser_clean());
        ASSERT_EQ(h.module_.size(), 1u);
        auto decl = std::dynamic_pointer_cast<CStructDecl>(h.module_[0]);
        ASSERT_NE(decl, nullptr);
        EXPECT_EQ(h.text(decl), "struct Point { int x; unsigned long y; };");
        EXPECT_EQ(decl->location().begin, 2u);
        EXPECT_EQ(decl->location().end, h.source_.size() - 1);
        auto type = decl->get_annotation();
        ASSERT_NE(type, nullptr);
        EXPECT_TRUE(type->has_definition());
        EXPECT_EQ(type->get_base_kind(), BaseAnnotationKind::Struct);
        EXPECT_EQ(h.text(type), "struct Point { int x; unsigned long y; }");
        EXPECT_EQ(h.text(type->get_tag()), "Point");
        const auto& fields = type->get_fields();
        ASSERT_EQ(fields.size(), 2u);
        ASSERT_NE(fields[0], nullptr);
        ASSERT_NE(fields[1], nullptr);
        EXPECT_EQ(h.text(fields[0]->get_name()), "x");
        EXPECT_EQ(h.text(fields[1]->get_name()), "y");
        EXPECT_EQ(h.text(fields[0]), "int x;");
        EXPECT_EQ(h.text(fields[1]), "unsigned long y;");
        EXPECT_EQ(fields[0]->get_initializer(), nullptr);
        EXPECT_FALSE(fields[0]->is_vararg());
        auto x_type =
            std::dynamic_pointer_cast<Primitive>(fields[0]->get_annotation());
        ASSERT_NE(x_type, nullptr);
        ASSERT_EQ(x_type->get_specifierc(), 1u);
        EXPECT_EQ(x_type->get_specifier_at(0).kind, CTokenKind::Int);
        auto y_type =
            std::dynamic_pointer_cast<Primitive>(fields[1]->get_annotation());
        ASSERT_NE(y_type, nullptr);
        ASSERT_EQ(y_type->get_specifierc(), 2u);
        EXPECT_EQ(y_type->get_specifier_at(0).kind, CTokenKind::Unsigned);
        EXPECT_EQ(y_type->get_specifier_at(1).kind, CTokenKind::Long);
    }

    TEST(CParserTest, StructDefinitionsWithObjectsAndInitializers) {
        for (auto src: {"struct Point { int x; } p = {2};",
                        "struct { int x; } p = {2};"}) {
            SCOPED_TRACE(src);
            Harness h{src};
            ASSERT_TRUE(h.upstream_clean());
            ASSERT_TRUE(h.parser_clean());
            ASSERT_EQ(h.module_.size(), 1u);
            auto decl = h.declaration();
            ASSERT_NE(decl, nullptr);
            EXPECT_EQ(h.text(decl->get_identifier()), "p");
            auto type =
                std::dynamic_pointer_cast<CStruct>(decl->get_annotation());
            ASSERT_NE(type, nullptr);
            EXPECT_TRUE(type->has_definition());
            EXPECT_EQ(type->get_tag() != nullptr,
                      h.source_.starts_with("struct Point"));
            ASSERT_EQ(type->get_fields().size(), 1u);
            auto list = std::dynamic_pointer_cast<InitializerList>(
                decl->get_initializer());
            ASSERT_NE(list, nullptr);
            ASSERT_EQ(list->get_elementc(), 1u);
            EXPECT_EQ(h.text(list->get_element_at(0)), "2");
        }
    }

    TEST(CParserTest, StructPointerArrayAndSelfReferencingFields) {
        Harness h{
            "struct Node { int **p; int a[2][3]; struct Node *next; }; struct "
            "Node nodes[4];"};
        ASSERT_TRUE(h.upstream_clean());
        ASSERT_TRUE(h.parser_clean());
        ASSERT_EQ(h.module_.size(), 2u);
        auto decl = std::dynamic_pointer_cast<CStructDecl>(h.module_[0]);
        ASSERT_NE(decl, nullptr);
        const auto& fields = decl->get_annotation()->get_fields();
        ASSERT_EQ(fields.size(), 3u);
        auto outer_ptr =
            std::dynamic_pointer_cast<Pointer>(fields[0]->get_annotation());
        ASSERT_NE(outer_ptr, nullptr);
        auto inner_ptr =
            std::dynamic_pointer_cast<Pointer>(outer_ptr->get_pointee());
        ASSERT_NE(inner_ptr, nullptr);
        EXPECT_NE(
            std::dynamic_pointer_cast<Primitive>(inner_ptr->get_pointee()),
            nullptr);
        auto outer_array =
            std::dynamic_pointer_cast<Array>(fields[1]->get_annotation());
        ASSERT_NE(outer_array, nullptr);
        EXPECT_EQ(h.text(outer_array->get_num_elements()), "2");
        auto inner_array =
            std::dynamic_pointer_cast<Array>(outer_array->get_element());
        ASSERT_NE(inner_array, nullptr);
        EXPECT_EQ(h.text(inner_array->get_num_elements()), "3");
        auto next =
            std::dynamic_pointer_cast<Pointer>(fields[2]->get_annotation());
        ASSERT_NE(next, nullptr);
        auto reference =
            std::dynamic_pointer_cast<CStruct>(next->get_pointee());
        ASSERT_NE(reference, nullptr);
        EXPECT_FALSE(reference->has_definition());
        EXPECT_TRUE(reference->get_fields().empty());
        EXPECT_EQ(h.text(reference->get_tag()), "Node");
        auto nodes = h.declaration(1);
        ASSERT_NE(nodes, nullptr);
        auto array = std::dynamic_pointer_cast<Array>(nodes->get_annotation());
        ASSERT_NE(array, nullptr);
        EXPECT_EQ(h.text(array->get_num_elements()), "4");
        EXPECT_NE(std::dynamic_pointer_cast<CStruct>(array->get_element()),
                  nullptr);
    }

    TEST(CParserTest, StructNestedDefinitions) {
        Harness h{
            "struct Outer { struct Inner { int x; } inner; struct { char c; } "
            "unnamed; };"};
        ASSERT_TRUE(h.upstream_clean());
        ASSERT_TRUE(h.parser_clean());
        ASSERT_EQ(h.module_.size(), 1u);
        auto decl = std::dynamic_pointer_cast<CStructDecl>(h.module_[0]);
        ASSERT_NE(decl, nullptr);
        const auto& fields = decl->get_annotation()->get_fields();
        ASSERT_EQ(fields.size(), 2u);
        auto named =
            std::dynamic_pointer_cast<CStruct>(fields[0]->get_annotation());
        ASSERT_NE(named, nullptr);
        EXPECT_TRUE(named->has_definition());
        EXPECT_EQ(h.text(named->get_tag()), "Inner");
        ASSERT_EQ(named->get_fields().size(), 1u);
        EXPECT_EQ(h.text(named->get_fields()[0]->get_name()), "x");
        auto unnamed =
            std::dynamic_pointer_cast<CStruct>(fields[1]->get_annotation());
        ASSERT_NE(unnamed, nullptr);
        EXPECT_TRUE(unnamed->has_definition());
        EXPECT_EQ(unnamed->get_tag(), nullptr);
        ASSERT_EQ(unnamed->get_fields().size(), 1u);
        EXPECT_EQ(h.text(unnamed->get_fields()[0]->get_name()), "c");
    }

    TEST(CParserTest, StructFunctionTypesAndLocalDeclarations) {
        Harness h{
            "struct Point f(struct Point p, struct Point *ptr) { struct Local "
            "{ int x; }; struct Point local; return p; }"};
        ASSERT_TRUE(h.upstream_clean());
        ASSERT_TRUE(h.parser_clean());
        auto fn = h.function();
        ASSERT_NE(fn, nullptr);
        auto ret = std::dynamic_pointer_cast<CStruct>(fn->get_return_type());
        ASSERT_NE(ret, nullptr);
        EXPECT_EQ(h.text(ret->get_tag()), "Point");
        ASSERT_EQ(fn->get_argc(), 2u);
        EXPECT_NE(std::dynamic_pointer_cast<CStruct>(
                      fn->get_arg_at(0)->get_annotation()),
                  nullptr);
        auto pointer = std::dynamic_pointer_cast<Pointer>(
            fn->get_arg_at(1)->get_annotation());
        ASSERT_NE(pointer, nullptr);
        EXPECT_NE(std::dynamic_pointer_cast<CStruct>(pointer->get_pointee()),
                  nullptr);
        auto block = h.body();
        ASSERT_NE(block, nullptr);
        ASSERT_EQ(block->statementc(), 3u);
        auto local_type = std::dynamic_pointer_cast<
            Statements::DeclarationStatement<COffsetType>>(
            block->get_statement(0));
        ASSERT_NE(local_type, nullptr);
        EXPECT_NE(std::dynamic_pointer_cast<CStructDecl>(local_type->get()),
                  nullptr);
        auto local = std::dynamic_pointer_cast<
            Statements::DeclarationStatement<COffsetType>>(
            block->get_statement(1));
        ASSERT_NE(local, nullptr);
        auto object = std::dynamic_pointer_cast<Let>(local->get());
        ASSERT_NE(object, nullptr);
        EXPECT_EQ(h.text(object->get_identifier()), "local");
    }

    TEST(CParserTest,
         StructEmptyBodiesReportErrorsAndPreserveFollowingGlobals) {
        for (auto src:
             {"struct Empty {}; int good;", "struct {} object; int good;"}) {
            SCOPED_TRACE(src);
            Harness h{src};
            ASSERT_TRUE(h.upstream_clean());
            auto diags = h.get_diags();
            ASSERT_EQ(diags.size(), 1u);
            EXPECT_EQ(diags[0].kind(),
                      CParserDiagnosticKind::ErrorExpectedType);
            EXPECT_EQ(diags[0].range().begin, h.source_.find('}'));
            ASSERT_EQ(h.module_.size(), 2u);
            auto good = h.declaration(1);
            ASSERT_NE(good, nullptr);
            EXPECT_EQ(h.text(good->get_identifier()), "good");
        }
    }

    TEST(CParserTest, StructMalformedMembersDoNotBecomeGlobals) {
        for (auto member:
             {"int broken = ;", "int broken[2 + ];", "unknown broken;",
              "struct ;", "int a, ;", "unsigned bits : 3;", "int function();",
              "union U { int hidden; } broken = ;"}) {
            SCOPED_TRACE(member);
            Harness h{std::string("struct S { ") + member +
                      " int kept; }; int good;"};
            ASSERT_TRUE(h.upstream_clean());
            EXPECT_FALSE(h.parser_clean());
            ASSERT_EQ(h.module_.size(), 2u);
            auto decl = std::dynamic_pointer_cast<CStructDecl>(h.module_[0]);
            ASSERT_NE(decl, nullptr);
            const auto& fields = decl->get_annotation()->get_fields();
            ASSERT_EQ(fields.size(), 1u);
            EXPECT_EQ(h.text(fields[0]->get_name()), "kept");
            auto good = h.declaration(1);
            ASSERT_NE(good, nullptr);
            EXPECT_EQ(h.text(good->get_identifier()), "good");
        }
    }

    TEST(CParserTest, StructRecoveryTracksNestedBracesAndConsumedDelimiters) {
        for (auto member:
             {"int broken = {{1,2}, {3,4}};",
              "int broken = { struct Hidden { int hidden; } value; };",
              "int broken[({ int hidden; })];"}) {
            SCOPED_TRACE(member);
            Harness h{std::string("struct S { ") + member +
                      " int kept; }; int good;"};
            ASSERT_TRUE(h.upstream_clean());
            EXPECT_FALSE(h.parser_clean());
            ASSERT_EQ(h.module_.size(), 2u);
            auto decl = std::dynamic_pointer_cast<CStructDecl>(h.module_[0]);
            ASSERT_NE(decl, nullptr);
            const auto& fields = decl->get_annotation()->get_fields();
            ASSERT_EQ(fields.size(), 1u);
            EXPECT_EQ(h.text(fields[0]->get_name()), "kept");
            auto good = h.declaration(1);
            ASSERT_NE(good, nullptr);
            EXPECT_EQ(h.text(good->get_identifier()), "good");
        }
    }

    TEST(CParserTest, StructMissingMemberSemicolonPreservesNextField) {
        Harness h{"struct S { int broken int kept; }; int good;"};
        ASSERT_TRUE(h.upstream_clean());
        auto diags = h.get_diags();
        ASSERT_EQ(diags.size(), 1u);
        EXPECT_EQ(diags[0].kind(), CParserDiagnosticKind::ErrorExpectedToken);
        ASSERT_EQ(h.module_.size(), 2u);
        auto decl = std::dynamic_pointer_cast<CStructDecl>(h.module_[0]);
        ASSERT_NE(decl, nullptr);
        const auto& fields = decl->get_annotation()->get_fields();
        ASSERT_EQ(fields.size(), 1u);
        EXPECT_EQ(h.text(fields[0]->get_name()), "kept");
    }

    TEST(CParserTest, StructRecoveryLeavesClosingBraceForItsOwner) {
        for (auto member: {"int broken", "int broken[", "int broken("}) {
            SCOPED_TRACE(member);
            Harness h{std::string("struct S { ") + member + " }; int good;"};
            ASSERT_TRUE(h.upstream_clean());
            EXPECT_FALSE(h.parser_clean());
            ASSERT_EQ(h.module_.size(), 2u);
            EXPECT_NE(std::dynamic_pointer_cast<CStructDecl>(h.module_[0]),
                      nullptr);
            auto good = h.declaration(1);
            ASSERT_NE(good, nullptr);
            EXPECT_EQ(h.text(good->get_identifier()), "good");
        }
    }

    TEST(CParserTest, StructInvalidDeclarationsRequireNames) {
        for (auto src: {"struct;", "struct Point *;", "struct { int x; };"}) {
            SCOPED_TRACE(src);
            Harness h{src};
            ASSERT_TRUE(h.upstream_clean());
            EXPECT_FALSE(h.parser_clean());
            EXPECT_TRUE(h.module_.empty());
        }
    }

    TEST(CParserTest, StructTruncatedDefinitionsTerminate) {
        for (auto src: {"struct", "struct S", "struct S {", "struct S { int",
                        "struct S { int x;", "struct S { int x[",
                        "struct S { struct Inner { int x;",
                        "struct S { int broken = {{1"}) {
            SCOPED_TRACE(src);
            Harness h{src};
            ASSERT_TRUE(h.upstream_clean());
            EXPECT_FALSE(h.parser_clean());
            EXPECT_TRUE(h.module_.empty());
            for (const auto& diag: h.get_diags()) {
                EXPECT_LE(diag.range().begin, diag.range().end);
                EXPECT_LE(diag.range().end, h.source_.size());
            }
        }
    }

}  // namespace Z::Zaban::Tests
