#include <gtest/gtest.h>

#include <Z/Zaban/Langs/CLang/Lexer.hpp>
#include <string>
#include <string_view>
#include <vector>

#include "Z/Zaban/Langs/CLang/TokenKind.hpp"

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
        std::vector<CLexerTokenType> tokens_of(std::string_view src) {
            CLexerBufferType buffer = src;
            CLexer           lexer(buffer);
            lexer.scan();
            return lexer.finalize();
        }

        bool token_has(const CLexerTokenType& t, TokenFlags f) {
            return has(static_cast<TokenFlags>(t.flags), f);
        }

        /// Arbitrary splits must never produce a diagnostic or lose bytes.
        /// It does NOT assert token identity — real chunk boundaries are
        /// region-delimited and land at line starts, so mid-token splits are
        /// not a case we serve.
        void expect_split_is_clean(std::string_view src) {
            for (std::size_t at = 1; at < src.size(); ++at) {
                CLexerBufferType lhs_buf = src.substr(0, at);
                CLexerBufferType rhs_buf = src.substr(at);

                CLexer lhs(lhs_buf);
                CLexer rhs(rhs_buf, at);
                lhs.scan();
                rhs.scan();
                lhs << rhs;

                const std::vector<CLexerTokenType> t = lhs.finalize();

                EXPECT_FALSE(lhs.diagnostics().has_errors())
                    << "split at " << at;
                ASSERT_FALSE(t.empty()) << "split at " << at;
                EXPECT_EQ(t.back().kind, CLexerTokenKind::Eob)
                    << "split at " << at;
                EXPECT_EQ(t.back().range.end, src.size())
                    << "split at " << at
                    << ": stream does not cover the source";
            }
        }
        /// Region-delimited chunks always break at a line start. This is the
        /// real chunking contract, so here token identity IS required.
        void expect_line_split_invariant(std::string_view src) {
            const std::vector<CLexerTokenKind> whole = kinds_of(src);

            for (std::size_t at = 0; at < src.size(); ++at) {
                if ('\n' != src[at]) continue;
                const std::size_t cut = at + 1;
                if (cut >= src.size()) continue;

                CLexerBufferType lhs_buf = src.substr(0, cut);
                CLexerBufferType rhs_buf = src.substr(cut);

                CLexer lhs(lhs_buf);
                CLexer rhs(rhs_buf, cut);
                lhs.scan();
                rhs.scan();
                lhs << rhs;

                std::vector<CLexerTokenKind> actual;
                for (const auto& t: lhs.finalize()) {
                    actual.push_back(t.kind);
                }

                ASSERT_EQ(actual.size(), whole.size())
                    << "line split at " << cut
                    << "\n  whole: " << describe(whole)
                    << "\n  split: " << describe(actual);
                for (std::size_t i = 0; i < whole.size(); ++i) {
                    EXPECT_EQ(actual[i], whole[i]) << "line split at " << cut;
                }
            }
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
    TEST(CLexerTest, ConcatFusesSplitExponentSign) {
        CLexerBufferType lhs_buf = "1e";
        CLexerBufferType rhs_buf = "-9";

        CLexer lhs(lhs_buf);
        CLexer rhs(rhs_buf, lhs_buf.size());

        lhs.scan();
        rhs.scan();
        lhs << rhs;

        const std::vector<CLexerTokenType> tokens = lhs.finalize();

        std::vector<CLexerTokenKind> actual;
        for (const auto& t: tokens) {
            actual.push_back(t.kind);
        }

        const std::vector<CLexerTokenKind> expected = {
            CLexerTokenKind::Numeric,
            CLexerTokenKind::Eob,
        };

        ASSERT_EQ(describe(actual), describe(expected));

        EXPECT_EQ(tokens[0].range.begin, 0u);
        EXPECT_EQ(tokens[0].range.end, 4u);
    }

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
    /**
     * Expect: `in\`+LF+`t` is the keyword int, not two identifiers.
     * Should: one Int token spanning all four bytes.
     */
    TEST(CLexerSpliceTest, SpliceInsideKeyword) {
        const std::vector<CLexerTokenType> t = tokens_of("in\\\nt x;");

        ASSERT_GE(t.size(), 1u);
        EXPECT_EQ(t[0].kind, CLexerTokenKind::Int);
        EXPECT_EQ(t[0].range.begin, 0u);
        EXPECT_EQ(t[0].range.end, 5u) << "range must cover the splice bytes";
        EXPECT_TRUE(token_has(t[0], TokenFlags::ContainsSplice));
    }

    /**
     * Expect: a splice inside a numeric literal does not end it.
     * Should: one Numeric token, value 42.
     */
    TEST(CLexerSpliceTest, SpliceInsideNumber) {
        expect_kinds("4\\\n2;",
                     {CLexerTokenKind::Numeric, CLexerTokenKind::Semicolon,
                      CLexerTokenKind::Eob});
    }

    /**
     * Expect: a splice between `/` and `*` still opens a block comment.
     * Should: the whole thing is trivia, only Eob survives.
     */
    TEST(CLexerSpliceTest, SpliceFormsCommentOpener) {
        expect_kinds("/\\\n* body *\\\n/", {CLexerTokenKind::Eob});
    }

    /**
     * Expect: a splice inside a string literal does not terminate it.
     * Should: one String token, no UnterminatedString error.
     */
    TEST(CLexerSpliceTest, SpliceInsideString) {
        CLexerBufferType buffer = "\"ab\\\ncd\";";
        CLexer           lexer(buffer);
        lexer.scan();

        const std::vector<CLexerTokenType> t = lexer.finalize();
        ASSERT_EQ(t.size(), 3u);
        EXPECT_EQ(t[0].kind, CLexerTokenKind::String);
        EXPECT_FALSE(lexer.diagnostics().has_errors());
    }

    /**
     * Expect: a line comment continued by a splice swallows the next line.
     * Should: `x` is commented out; only `y` reaches the stream.
     */
    TEST(CLexerSpliceTest, SpliceContinuesLineComment) {
        expect_kinds("// c \\\nx\ny;",
                     {CLexerTokenKind::Identifier, CLexerTokenKind::Semicolon,
                      CLexerTokenKind::Eob});
    }

    /**
     * Expect: a real backslash (not followed by a newline) is not a splice.
     * Should: it survives to lex_punctuator's default path.
     */
    TEST(CLexerSpliceTest, LoneBackslashIsNotSpliced) {
        const std::vector<CLexerTokenKind> k = kinds_of("a\\b");
        EXPECT_EQ(k.front(), CLexerTokenKind::Identifier);
        EXPECT_GT(k.size(), 2u) << describe(k);
    }
    /**
     * Expect: the spec's torture case reduces to exactly `#define FOO 1020`.
     * Should: Hash, Identifier(define), Identifier(FOO), Numeric, Eob.
     */
    TEST(CLexerSpliceTest, CursedDirective) {
        static constexpr std::string_view src =
            "/\\\n"
            "*\n"
            "*/ # /*\n"
            "*/ defi\\\n"
            "ne FO\\\n"
            "O 10\\\n"
            "20\n";

        expect_kinds(src, {CLexerTokenKind::Hash, CLexerTokenKind::Identifier,
                           CLexerTokenKind::Identifier,
                           CLexerTokenKind::Numeric, CLexerTokenKind::Eob});
    }
    /**
     * Expect: AtLineStart marks the first token after a real line ending.
     * Should: set on `#`, clear on `define`.
     */
    TEST(CLexerFlagTest, AtLineStart) {
        const std::vector<CLexerTokenType> t = tokens_of("x;\n#define");

        ASSERT_GE(t.size(), 4u) << "got " << t.size() << " tokens";
        EXPECT_TRUE(token_has(t[0], TokenFlags::AtLineStart)) << "x";
        EXPECT_TRUE(token_has(t[2], TokenFlags::AtLineStart)) << "#";
        EXPECT_FALSE(token_has(t[3], TokenFlags::AtLineStart)) << "define";
    }
    /**
     * Expect: only the chunk owning byte 0 grants start-of-source AtLineStart.
     * Should: a mid-file chunk's first token is not at line start.
     */
    TEST(CLexerFlagTest, MidFileChunkIsNotLineStart) {
        CLexerBufferType buffer = "y;";
        CLexer           lexer(buffer, 500);
        lexer.scan();

        const std::vector<CLexerTokenType> t = lexer.finalize();
        ASSERT_GE(t.size(), 1u);
        EXPECT_FALSE(token_has(t[0], TokenFlags::AtLineStart));
    }
    /**
     * Expect: a token after `\`+newline is NOT at line start.
     * Should: this is why splicing lives below skip_trivia.
     */
    TEST(CLexerFlagTest, SpliceIsNotLineStart) {
        const std::vector<CLexerTokenType> t = tokens_of("x;\\\ny");

        ASSERT_GE(t.size(), 3u);
        EXPECT_FALSE(token_has(t[2], TokenFlags::AtLineStart));
    }

    /**
     * Expect: WhiteSpaceBefore is set by a comment, not only by spaces.
     * Should: `F/**\/(x)` is object-like, so `(` carries the flag.
     */
    TEST(CLexerFlagTest, CommentSetsWhitespaceBefore) {
        const std::vector<CLexerTokenType> t = tokens_of("F/**/(x)");

        ASSERT_GE(t.size(), 2u);
        EXPECT_FALSE(token_has(t[0], TokenFlags::WhiteSpaceBefore)) << "F";
        EXPECT_TRUE(token_has(t[1], TokenFlags::WhiteSpaceBefore)) << "(";
    }

    /**
     * Expect: adjacent tokens carry no WhiteSpaceBefore.
     * Should: `F(x)` is function-like.
     */
    TEST(CLexerFlagTest, NoWhitespaceBetweenAdjacentTokens) {
        const std::vector<CLexerTokenType> t = tokens_of("F(x)");

        ASSERT_GE(t.size(), 2u);
        EXPECT_FALSE(token_has(t[1], TokenFlags::WhiteSpaceBefore));
    }
    /**
     * Expect: CRLF, bare CR, and bare LF all behave as one line ending.
     * Should: identical token streams across all three.
     */
    TEST(CLexerLineEndingTest, MixedLineEndingsAgree) {
        const std::vector<CLexerTokenKind> lf   = kinds_of("a\nb");
        const std::vector<CLexerTokenKind> crlf = kinds_of("a\r\nb");
        const std::vector<CLexerTokenKind> cr   = kinds_of("a\rb");

        EXPECT_EQ(describe(crlf), describe(lf));
        EXPECT_EQ(describe(cr), describe(lf));
    }
    TEST(CLexerSplitTest, SpliceInsideKeywordAtEverySplit) {
        expect_split_is_clean("in\\\nt x = 4\\\n2;");
    }

    TEST(CLexerSplitTest, CursedDirectiveAtEverySplit) {
        expect_line_split_invariant(
            "/\\\n*\n*/ # /*\n*/ defi\\\nne FO\\\nO 10\\\n20\n");
    }

    TEST(CLexerSplitTest, PlainSourceAtEverySplit) {
        expect_split_is_clean("int x = 42; x >>= 1; /* c */ y++;");
    }

    TEST(CLexerSplitTest, BracketDigraphs) {
        expect_kinds(
            "arr<:3:>",
            {CLexerTokenKind::Identifier, CLexerTokenKind::LBrak,
             CLexerTokenKind::Numeric, CLexerTokenKind::RBrak, TokenKind::Eob});
    }

    TEST(CLexerSplitTest, BraceDigraphs) {
        expect_kinds("<%1%>",
                     {CLexerTokenKind::LBrace, CLexerTokenKind::Numeric,
                      CLexerTokenKind::RBrace, CLexerTokenKind::Eob});
    }

    TEST(CLexerSplitTest, HashDigraph) {
        const std::vector<CLexerTokenType> t = tokens_of("%:define x 1");
        ASSERT_GE(t.size(), 1u);
        EXPECT_EQ(t[0].kind, CLexerTokenKind::Hash);
        EXPECT_EQ(t[0].range.end - t[0].range.begin, 2u)
            << "digraph Hash spans two bytes";
        EXPECT_TRUE(token_has(t[0], TokenFlags::AtLineStart));
    }
    TEST(CLexerDigraphTest, HashHashDigraph) {
        expect_kinds("a%:%:b",
                     {CLexerTokenKind::Identifier, CLexerTokenKind::HashHash,
                      CLexerTokenKind::Identifier, CLexerTokenKind::Eob});
    }

    TEST(CLexerDigraphTest, SplicedDigraph) {
        expect_kinds("%\\\n:define",
                     {CLexerTokenKind::Hash, CLexerTokenKind::Identifier,
                      CLexerTokenKind::Eob});
    }

    TEST(CLexerSplitTest, DigraphsAtEverySplit) {
        expect_split_is_clean("arr<:3:> = <%1, 2%>; %:define X 1");
    }
}  // namespace Z::Zaban::Tests
