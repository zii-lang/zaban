#include <gtest/gtest.h>

#include <Z/Zaban/Langs/CLang/Lexer.hpp>
#include <Z/Zaban/Langs/CLang/Preprocessor.hpp>
#include <string>
#include <string_view>
#include <vector>

namespace Z::Zaban::Tests {
    using namespace Z::Zaban::Langs::CLang;

    namespace {
        /// Lexes then preprocesses, the way the driver will.
        std::vector<CLexerTokenType> pp_tokens(std::string_view src) {
            CLexerBufferType buffer = src;
            CLexer           lexer(buffer);
            lexer.scan();

            CPreprocessor pp(buffer);
            return pp.process(lexer.finalize());
        }

        bool token_has(const CLexerTokenType& t, TokenFlags f) {
            return has(static_cast<TokenFlags>(t.flags), f);
        }

        std::size_t count_flagged(const std::vector<CLexerTokenType>& tokens,
                                  TokenFlags                          f) {
            std::size_t n = 0;
            for (const auto& t: tokens) {
                if (token_has(t, f)) ++n;
            }
            return n;
        }

        std::string describe(const std::vector<CLexerTokenType>& tokens) {
            std::string out;
            for (std::size_t i = 0; i < tokens.size(); ++i) {
                if (i > 0) out += ", ";
                out += to_string(tokens[i].kind);
                if (token_has(tokens[i], TokenFlags::DirectiveLine)) {
                    out += "*";
                }
            }
            return out;
        }
    }  // namespace
    /**
     * Expect: WhiteSpaceBefore distinguishes function-like from object-like.
     * Should: `F(x)` has no space before `(`; `F (x)` does. This is the only
     * thing that separates the two forms, and it is unrecoverable after
     * lexing.
     */
    TEST(CPreprocessorTest, ObjectLikeVsFunctionLike) {
        const std::vector<CLexerTokenType> fn  = pp_tokens("#define F(x) x");
        const std::vector<CLexerTokenType> obj = pp_tokens("#define F (x) x");

        ASSERT_GE(fn.size(), 4u) << describe(fn);
        ASSERT_GE(obj.size(), 4u) << describe(obj);

        ASSERT_EQ(fn[3].kind, CLexerTokenKind::LParen);
        ASSERT_EQ(obj[3].kind, CLexerTokenKind::LParen);

        EXPECT_FALSE(token_has(fn[3], TokenFlags::WhiteSpaceBefore))
            << "function-like";
        EXPECT_TRUE(token_has(obj[3], TokenFlags::WhiteSpaceBefore))
            << "object-like";
    }
    /**
     * Expect: every token on the directive line is marked, and nothing else.
     * Should: `#define F 1` marks 4 tokens; the trailing `int x;` is untouched.
     */
    TEST(CPreprocessorTest, MarksDirectiveLineOnly) {
        const std::vector<CLexerTokenType> t = pp_tokens("#define F 1\nint x;");

        EXPECT_EQ(count_flagged(t, TokenFlags::DirectiveLine), 4u)
            << describe(t);
        EXPECT_TRUE(token_has(t[0], TokenFlags::DirectiveLine)) << "#";
        EXPECT_TRUE(token_has(t[3], TokenFlags::DirectiveLine)) << "1";
        EXPECT_FALSE(token_has(t[4], TokenFlags::DirectiveLine)) << "int";
    }

    /**
     * Expect: a Hash not at line start is not a directive.
     * Should: the `#` in a macro body is left alone.
     */
    TEST(CPreprocessorTest, MidLineHashIsNotADirective) {
        const std::vector<CLexerTokenType> t = pp_tokens("int x; # y");

        EXPECT_EQ(count_flagged(t, TokenFlags::DirectiveLine), 0u)
            << describe(t);
    }

    /**
     * Expect: a directive may be indented.
     * Should: leading whitespace does not clear AtLineStart.
     */
    TEST(CPreprocessorTest, IndentedDirective) {
        const std::vector<CLexerTokenType> t = pp_tokens("  #define F 1");

        EXPECT_EQ(count_flagged(t, TokenFlags::DirectiveLine), 4u)
            << describe(t);
    }

    /**
     * Expect: a bare `#` on its own line is a null directive, still marked.
     */
    TEST(CPreprocessorTest, NullDirective) {
        const std::vector<CLexerTokenType> t = pp_tokens("#\nint x;");

        EXPECT_TRUE(token_has(t[0], TokenFlags::DirectiveLine));
        EXPECT_FALSE(token_has(t[1], TokenFlags::DirectiveLine)) << describe(t);
    }

    /**
     * Expect: two directives on consecutive lines are two separate lines.
     */
    TEST(CPreprocessorTest, ConsecutiveDirectives) {
        const std::vector<CLexerTokenType> t =
            pp_tokens("#define A 1\n#define B 2\n");

        EXPECT_EQ(count_flagged(t, TokenFlags::DirectiveLine), 8u)
            << describe(t);
    }
    /**
     * Expect: `%:` is a directive opener exactly like `#`.
     * Should: the digraph merges to Hash and keeps AtLineStart.
     */
    TEST(CPreprocessorTest, DigraphDirective) {
        const std::vector<CLexerTokenType> t = pp_tokens("%:define F 1");

        ASSERT_GE(t.size(), 1u);
        EXPECT_EQ(t[0].kind, CLexerTokenKind::Hash);
        EXPECT_EQ(count_flagged(t, TokenFlags::DirectiveLine), 4u)
            << describe(t);
    }

    /**
     * Expect: a directive line continued by a splice is one logical line.
     * Should: the tokens after the splice stay on the directive line, because
     * splicing removed the newline before AtLineStart could be set.
     */
    TEST(CPreprocessorTest, SplicedDirectiveContinuation) {
        const std::vector<CLexerTokenType> t =
            pp_tokens("#define F \\\n1\nint x;");

        EXPECT_EQ(count_flagged(t, TokenFlags::DirectiveLine), 4u)
            << describe(t);
        EXPECT_FALSE(token_has(t[4], TokenFlags::DirectiveLine)) << "int";
    }

    /**
     * Expect: the cursed torture case is exactly one directive line.
     */
    TEST(CPreprocessorTest, CursedDirective) {
        static constexpr std::string_view src =
            "/\\\n"
            "*\n"
            "*/ # /*\n"
            "*/ defi\\\n"
            "ne FO\\\n"
            "O 10\\\n"
            "20\n";

        const std::vector<CLexerTokenType> t = pp_tokens(src);

        EXPECT_EQ(count_flagged(t, TokenFlags::DirectiveLine), 4u)
            << describe(t);
    }
    /**
     * Expect: process() currently changes nothing but flags.
     * Should: same token count and kinds in, same out.
     */
    TEST(CPreprocessorTest, PreservesStream) {
        CLexerBufferType buffer = "#define F 1\nint x = F;";
        CLexer           lexer(buffer);
        lexer.scan();
        const std::vector<CLexerTokenType> before = lexer.finalize();

        CPreprocessor                      pp(buffer);
        const std::vector<CLexerTokenType> after = pp.process(before);

        ASSERT_EQ(after.size(), before.size());
        for (std::size_t i = 0; i < before.size(); ++i) {
            EXPECT_EQ(after[i].kind, before[i].kind) << "index " << i;
            EXPECT_EQ(after[i].range.begin, before[i].range.begin);
        }
    }

}  // namespace Z::Zaban::Tests
