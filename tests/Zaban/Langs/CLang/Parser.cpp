#include <gtest/gtest.h>

#include <Z/Zaban/Langs/CLang/Lexer.hpp>
#include <Z/Zaban/Langs/CLang/Parser.hpp>
#include <Z/Zaban/Langs/CLang/Preprocessor.hpp>
#include <utility>

#include "Z/Zaban/AST/Node.hpp"
#include "Z/Zaban/Langs/CLang/ParserDiagnostic.hpp"
#include "Z/Zaban/Lex/LexerDiagnostic.hpp"
#include "gtest/gtest.h"

namespace Z::Zaban::Tests {
    using namespace Z::Zaban::Langs::CLang;
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

            /// Use as ASSERT_TRUE(h.upstream_clean()) before checking the
            /// parser. lists every lexer/preprocessor error on failure.
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
        ASSERT_EQ(h.module_[0]->node_kind(), AST::NodeKind::Declaration);
        ASSERT_EQ(h.module_[1]->node_kind(), AST::NodeKind::Declaration);
        h.print_module();
    }

    TEST(CParserTest, SimpleDeclaration) {
        Harness h{"int x; "};
        ASSERT_TRUE(h.upstream_clean());
        ASSERT_TRUE(h.parser_clean());
        h.print_module();
    }

}  // namespace Z::Zaban::Tests
