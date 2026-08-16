#include <gtest/gtest.h>

#include <Z/Zaban/Langs/CLang/Lexer.hpp>
#include <string>
#include <string_view>
#include <vector>

namespace Z::Zaban::Tests {
    using namespace Z::Zaban::Langs::CLang;

    namespace {
        std::vector<CLexerTokenKind> kinds_of(std::string_view src) {
            CLexerBufferType buffer = src;
            CLexer           lexer(buffer);

            lexer.scan();

            std::vector<CLexerTokenKind> kinds;
            for (const auto& t: lexer.finalize()) {
                kinds.push_back(t.kind);
            }
            return kinds;
        }

        void expect_kinds(std::string_view                    src,
                          const std::vector<CLexerTokenKind>& expected) {
            const std::vector<CLexerTokenKind> actual = kinds_of(src);

            ASSERT_EQ(actual.size(), expected.size()) << "source: " << src;
            for (std::size_t i = 0; i < expected.size(); ++i) {
                EXPECT_EQ(actual[i], expected[i])
                    << "index " << i << " in source: " << src;
            }
        }
        /// Renders a token stream as "Int, Identifier, Semicolon, Eob" for
        /// failure messages.
        std::string describe(const std::vector<CLexerTokenKind>& kinds) {
            std::string out;
            for (std::size_t i = 0; i < kinds.size(); ++i) {
                if (i > 0) {
                    out += ", ";
                }
                out += to_string(kinds[i]);
            }
            return out;
        }
    }  // namespace

    /**
     * Expect: an empty buffer produces only the eof marker.
     * Should: not fail, one Eob token.
     */
    TEST(CLexerTest, ScanEmptySource) {
        expect_kinds("", {CLexerTokenKind::Eob});
    }

    /**
     * Expect: whitespace and comments are trivia, never tokens.
     * Should: produce a single identifier followed by Eob.
     */
    TEST(CLexerTest, ScanSkipsTrivia) {
        expect_kinds("  \t\n// line comment\n/* block\ncomment */ x",
                     {CLexerTokenKind::Identifier, CLexerTokenKind::Eob});
    }

    /**
     * Expect: keywords are recognized, non-keywords stay identifiers.
     * Should: map spellings and underscore-prefixed aliases to the same kind.
     */
    TEST(CLexerTest, ScanKeywordsAndIdentifiers) {
        expect_kinds("int _Bool bool intx static_assert",
                     {
                         CLexerTokenKind::Int,
                         CLexerTokenKind::Bool,
                         CLexerTokenKind::Bool,
                         CLexerTokenKind::Identifier,
                         CLexerTokenKind::StaticAssert,
                         CLexerTokenKind::Eob,
                     });
    }

    /**
     * Expect: pp-number greediness, including signed exponents.
     * Should: each literal is exactly one Numeric token.
     */
    TEST(CLexerTest, ScanNumericLiterals) {
        expect_kinds("0 42 3.14 .5 0xFFull 1e-9 0x1p+3",
                     {
                         CLexerTokenKind::Numeric,
                         CLexerTokenKind::Numeric,
                         CLexerTokenKind::Numeric,
                         CLexerTokenKind::Numeric,
                         CLexerTokenKind::Numeric,
                         CLexerTokenKind::Numeric,
                         CLexerTokenKind::Numeric,
                         CLexerTokenKind::Eob,
                     });
    }

    /**
     * Expect: string and char literals, including escaped delimiters.
     * Should: one token per literal, escapes do not terminate early.
     */
    TEST(CLexerTest, ScanStringAndCharLiterals) {
        expect_kinds(R"("hi" "a\"b" 'c' '\'' '\\')",
                     {
                         CLexerTokenKind::String,
                         CLexerTokenKind::String,
                         CLexerTokenKind::CharLiteral,
                         CLexerTokenKind::CharLiteral,
                         CLexerTokenKind::CharLiteral,
                         CLexerTokenKind::Eob,
                     });
    }

    /**
     * Expect: maximal munch on punctuators within one chunk.
     * Should: prefer the longest operator at each position.
     */
    TEST(CLexerTest, ScanMaximalMunchPunctuators) {
        expect_kinds(">>= >> > <<= -> ++ == != ...",
                     {
                         CLexerTokenKind::GreaterGreaterEqual,
                         CLexerTokenKind::GreaterGreater,
                         CLexerTokenKind::Greater,
                         CLexerTokenKind::LesserLesserEqual,
                         CLexerTokenKind::Arrow,
                         CLexerTokenKind::PlusPlus,
                         CLexerTokenKind::EqualEqual,
                         CLexerTokenKind::ExclamEqual,
                         CLexerTokenKind::Ellipsis,
                         CLexerTokenKind::Eob,
                     });
    }

    /**
     * Expect: a literal with no closing delimiter survives to finalize().
     * Should: normalize StringOpen to String rather than leaking the open
     * fragment kind to consumers.
     */
    TEST(CLexerTest, FinalizeNormalizesUnterminatedString) {
        expect_kinds("\"abc", {CLexerTokenKind::String, CLexerTokenKind::Eob});
    }

    /**
     * Expect: an identifier cut by a chunk boundary is fused.
     * Should: "foo" + "bar123" become one Identifier.
     */
    TEST(CLexerTest, ConcatFusesSplitIdentifier) {
        CLexerBufferType lhs_buf = "foo";
        CLexerBufferType rhs_buf = "bar123";

        CLexer lhs(lhs_buf);
        CLexer rhs(rhs_buf, lhs_buf.size());

        lhs.scan();
        rhs.scan();
        lhs << rhs;

        const std::vector<CLexerTokenType> tokens = lhs.finalize();

        ASSERT_EQ(tokens.size(), 2u);
        EXPECT_EQ(tokens[0].kind, CLexerTokenKind::Identifier);
        EXPECT_EQ(tokens[0].range.begin, 0u);
        EXPECT_EQ(tokens[0].range.end, 9u);
        EXPECT_EQ(tokens[1].kind, CLexerTokenKind::Eob);
    }

    /**
     * Expect: an operator cut by a chunk boundary is fused per maximal munch.
     * Should: ">" + ">=" become one GreaterGreaterEqual.
     */
    TEST(CLexerTest, ConcatFusesSplitOperator) {
        CLexerBufferType lhs_buf = ">";
        CLexerBufferType rhs_buf = ">=";

        CLexer lhs(lhs_buf);
        CLexer rhs(rhs_buf, lhs_buf.size());

        lhs.scan();
        rhs.scan();
        lhs << rhs;

        const std::vector<CLexerTokenType> tokens = lhs.finalize();

        ASSERT_EQ(tokens.size(), 2u);
        EXPECT_EQ(tokens[0].kind, CLexerTokenKind::GreaterGreaterEqual);
        EXPECT_EQ(tokens[1].kind, CLexerTokenKind::Eob);
    }

    /**
     * Expect: a string opened in one chunk and closed in the next is repaired.
     * Should: yield one String token spanning the seam, then the semicolon.
     */
    TEST(CLexerTest, ConcatRepairsSplitString) {
        CLexerBufferType lhs_buf = "\"he";
        CLexerBufferType rhs_buf = "llo\";";

        CLexer lhs(lhs_buf);
        CLexer rhs(rhs_buf, lhs_buf.size());

        lhs.scan();
        rhs.scan();
        lhs << rhs;

        const std::vector<CLexerTokenType> tokens = lhs.finalize();

        ASSERT_EQ(tokens.size(), 3u);
        EXPECT_EQ(tokens[0].kind, CLexerTokenKind::String);
        EXPECT_EQ(tokens[0].range.begin, 0u);
        EXPECT_EQ(tokens[0].range.end, 7u);
        EXPECT_EQ(tokens[1].kind, CLexerTokenKind::Semicolon);
        EXPECT_EQ(tokens[2].kind, CLexerTokenKind::Eob);
    }

    /**
     * Expect: splitting a source at an arbitrary point does not change the
     * token stream.
     * Should: chunked scan equal whole-buffer scan.
     */
    TEST(CLexerTest, ConcatMatchesWholeBufferScan) {
        const std::string_view whole = "int x = 42; x >>= 1;";

        const std::vector<CLexerTokenKind> expected = kinds_of(whole);

        CLexerBufferType lhs_buf = whole.substr(0, 15);
        CLexerBufferType rhs_buf = whole.substr(15);

        CLexer lhs(lhs_buf);
        CLexer rhs(rhs_buf, lhs_buf.size());

        lhs.scan();
        rhs.scan();
        lhs << rhs;

        std::vector<CLexerTokenKind> actual;
        for (const auto& t: lhs.finalize()) {
            actual.push_back(t.kind);
        }

        ASSERT_EQ(actual.size(), expected.size());
        for (std::size_t i = 0; i < expected.size(); ++i) {
            EXPECT_EQ(actual[i], expected[i]) << "index " << i;
        }
    }
    /**
     * Expect: a block comment wholly inside one chunk is trivia.
     * Should: pass today. Baseline for the split case below.
     */
    TEST(CLexerTest, ScanBlockCommentIsTrivia) {
        expect_kinds("int x; /* note */ int y;",
                     {
                         CLexerTokenKind::Int,
                         CLexerTokenKind::Identifier,
                         CLexerTokenKind::Semicolon,
                         CLexerTokenKind::Int,
                         CLexerTokenKind::Identifier,
                         CLexerTokenKind::Semicolon,
                         CLexerTokenKind::Eob,
                     });
    }

    /**
     * Expect: a block comment opened in one chunk and closed in the next is
     * still trivia, and contributes no tokens.
     * Should:
     */
    TEST(CLexerTest, ConcatBlockCommentSplitAcrossChunks) {
        const std::string_view whole = "int x; /* note */ int y;";

        CLexerBufferType lhs_buf = whole.substr(0, 12);  // "int x; /* no"
        CLexerBufferType rhs_buf = whole.substr(12);     // "te */ int y;"

        CLexer lhs(lhs_buf);
        CLexer rhs(rhs_buf, lhs_buf.size());

        lhs.scan();
        rhs.scan();
        lhs << rhs;

        std::vector<CLexerTokenKind> actual;
        for (const auto& t: lhs.finalize()) {
            actual.push_back(t.kind);
        }

        const std::vector<CLexerTokenKind> expected = {
            CLexerTokenKind::Int,        CLexerTokenKind::Identifier,
            CLexerTokenKind::Semicolon,  CLexerTokenKind::Int,
            CLexerTokenKind::Identifier, CLexerTokenKind::Semicolon,
            CLexerTokenKind::Eob,
        };

        EXPECT_EQ(describe(actual), describe(expected));
    }
    /**
     * Expect: a `*​/` whose two bytes fall on opposite sides of the seam
     * still closes the comment. Should: comment contributes no tokens.
     */
    TEST(CLexerTest, ConcatBlockCommentCloseStraddlesSeam) {
        const std::string_view whole = "int x; /* note */ int y;";

        CLexerBufferType lhs_buf = whole.substr(0, 16);  // ends on '*'
        CLexerBufferType rhs_buf = whole.substr(16);     // starts on '/'

        CLexer lhs(lhs_buf);
        CLexer rhs(rhs_buf, lhs_buf.size());

        lhs.scan();
        rhs.scan();
        lhs << rhs;

        std::vector<CLexerTokenKind> actual;
        for (const auto& t: lhs.finalize()) {
            actual.push_back(t.kind);
        }

        const std::vector<CLexerTokenKind> expected = {
            CLexerTokenKind::Int,        CLexerTokenKind::Identifier,
            CLexerTokenKind::Semicolon,  CLexerTokenKind::Int,
            CLexerTokenKind::Identifier, CLexerTokenKind::Semicolon,
            CLexerTokenKind::Eob,
        };

        EXPECT_EQ(describe(actual), describe(expected));
    }

    /**
     * Expect: a block comment with no closing delimiter at a true EOF.
     * Should: produce no tokens for the comment body. Documents current
     * behaviour, which happens to be correct for the single-chunk case.
     */
    TEST(CLexerTest, ScanUnterminatedBlockComment) {
        expect_kinds("int x; /* never closed", {
                                                   CLexerTokenKind::Int,
                                                   CLexerTokenKind::Identifier,
                                                   CLexerTokenKind::Semicolon,
                                                   CLexerTokenKind::Eob,
                                               });
    }
    /**
     * Expect: a line comment wholly inside one chunk is trivia.
     */
    TEST(CLexerTest, ScanLineCommentIsTrivia) {
        expect_kinds("int x; // note\nint y;", {
                                                   CLexerTokenKind::Int,
                                                   CLexerTokenKind::Identifier,
                                                   CLexerTokenKind::Semicolon,
                                                   CLexerTokenKind::Int,
                                                   CLexerTokenKind::Identifier,
                                                   CLexerTokenKind::Semicolon,
                                                   CLexerTokenKind::Eob,
                                               });
    }

    /**
     * Expect: a line comment opened in one chunk and closed by a newline in
     * the next contributes no tokens.
     * Should:
     */
    TEST(CLexerTest, ConcatLineCommentSplitAcrossChunks) {
        const std::string_view whole = "int x; // note\nint y;";

        CLexerBufferType lhs_buf = whole.substr(0, 12);  // "int x; // no"
        CLexerBufferType rhs_buf = whole.substr(12);     // "te\nint y;"

        CLexer lhs(lhs_buf);
        CLexer rhs(rhs_buf, lhs_buf.size());

        lhs.scan();
        rhs.scan();
        lhs << rhs;

        std::vector<CLexerTokenKind> actual;
        for (const auto& t: lhs.finalize()) {
            actual.push_back(t.kind);
        }

        const std::vector<CLexerTokenKind> expected = {
            CLexerTokenKind::Int,        CLexerTokenKind::Identifier,
            CLexerTokenKind::Semicolon,  CLexerTokenKind::Int,
            CLexerTokenKind::Identifier, CLexerTokenKind::Semicolon,
            CLexerTokenKind::Eob,
        };

        EXPECT_EQ(describe(actual), describe(expected));
    }

    /**
     * Expect: a line comment whose rhs chunk contains no newline stays open
     * across the whole chunk.
     * Should:
     */
    TEST(CLexerTest, ConcatLineCommentSpansWholeRhs) {
        const std::string_view whole = "int x; // aaaaaaaa";

        CLexerBufferType lhs_buf = whole.substr(0, 12);
        CLexerBufferType rhs_buf = whole.substr(12);

        CLexer lhs(lhs_buf);
        CLexer rhs(rhs_buf, lhs_buf.size());

        lhs.scan();
        rhs.scan();
        lhs << rhs;

        std::vector<CLexerTokenKind> actual;
        for (const auto& t: lhs.finalize()) {
            actual.push_back(t.kind);
        }

        const std::vector<CLexerTokenKind> expected = {
            CLexerTokenKind::Int,
            CLexerTokenKind::Identifier,
            CLexerTokenKind::Semicolon,
            CLexerTokenKind::Eob,
        };

        EXPECT_EQ(describe(actual), describe(expected));
    }
    /**
     * Expect: signed exponents inside one chunk are single Numeric tokens.
     */
    TEST(CLexerTest, ScanExponentSignSingleChunk) {
        expect_kinds("1e-9 2E+3 0x1p-2", {
                                             CLexerTokenKind::Numeric,
                                             CLexerTokenKind::Numeric,
                                             CLexerTokenKind::Numeric,
                                             CLexerTokenKind::Eob,
                                         });
    }

    /**
     * Expect: a number split immediately after its exponent prefix fuses back
     * into one Numeric.
     */
    // TODO: fix
    // TEST(CLexerTest, ConcatFusesSplitExponentSign) {
    //     CLexerBufferType lhs_buf = "1e";
    //     CLexerBufferType rhs_buf = "-9";

    //     CLexer lhs(lhs_buf);
    //     CLexer rhs(rhs_buf, lhs_buf.size());

    //     lhs.scan();
    //     rhs.scan();
    //     lhs << rhs;

    //     const std::vector<CLexerTokenType> tokens = lhs.finalize();

    //     ASSERT_EQ(tokens.size(), 2u);
    //     EXPECT_EQ(tokens[0].kind, CLexerTokenKind::Numeric);
    //     EXPECT_EQ(tokens[0].range.begin, 0u);
    //     EXPECT_EQ(tokens[0].range.end, 4u);
    //     EXPECT_EQ(tokens[1].kind, CLexerTokenKind::Eob);
    // }

    /**
     * Expect: a minus after a number NOT ending in an exponent prefix stays a
     * separate operator. "1" | "-9" is subtraction, not one literal.
     * Should:
     */
    TEST(CLexerTest, ConcatDoesNotFuseSubtractionAsExponent) {
        CLexerBufferType lhs_buf = "1";
        CLexerBufferType rhs_buf = "-9";

        CLexer lhs(lhs_buf);
        CLexer rhs(rhs_buf, lhs_buf.size());

        lhs.scan();
        rhs.scan();
        lhs << rhs;

        std::vector<CLexerTokenKind> actual;
        for (const auto& t: lhs.finalize()) {
            actual.push_back(t.kind);
        }

        const std::vector<CLexerTokenKind> expected = {
            CLexerTokenKind::Numeric,
            CLexerTokenKind::Minus,
            CLexerTokenKind::Numeric,
            CLexerTokenKind::Eob,
        };

        EXPECT_EQ(describe(actual), describe(expected));
    }
}  // namespace Z::Zaban::Tests
