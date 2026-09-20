#include <gtest/gtest.h>

#include <Z/Zaban/Langs/ZLang/Lexer.hpp>
#include <array>

namespace Z::Zaban::Tests {
    using namespace Z::Zaban::Langs::ZLang;

    static bool flagged(const ZLexerTokenType& t, TokenFlags f) {
        return has(static_cast<TokenFlags>(t.flags), f);
    }

    static void expect_token(std::string_view source, const ZLexerTokenType& t,
                             ZLexerTokenKind  expected_kind,
                             std::string_view expected_text) {
        EXPECT_EQ(t.kind, expected_kind);
        ASSERT_LE(t.range.begin, t.range.end);
        ASSERT_LE(t.range.end, source.size());
        EXPECT_EQ(source.substr(t.range.begin, t.range.end - t.range.begin),
                  expected_text);
    }
    /**
     * Expect: finalize lexer scan.
     * Should: not fail, there are no tokens only EOF.
     */
    TEST(ZLexer, ScanSingleLexerEmptySource) {
        std::string_view source = {""};

        ZLexer                       lexer(source);
        std::vector<ZLexerTokenType> tokens;
        EXPECT_NO_FATAL_FAILURE(tokens = lexer.finalize());

        EXPECT_EQ(1, tokens.size());
        ASSERT_FALSE(tokens.empty());

        const auto& token = tokens.back();
        EXPECT_EQ(ZLexerTokenKind::Eof, token.kind);
    }

    /**
     * Expect: All keywords and other tokens parsed.
     * Should: scan without fail, have all tokens scanned.
     */
    TEST(ZLexer, ScanSingleLexerAllTokens) {
        std::string_view source =
            "+ - * / % | & = ! ~ ^ () [] {} . .. , : :: ; ++ -- && &> || < <= "
            "<< > >= >> > >= >> += -= *= /= %= &= |= == != >>= <<= -> => := ? "
            "?& ?| ?? ?! !! @ @@ @: true false let type return struct enum if "
            "endif loop endloop func vari break continue goto label";

        std::array<ZLexerTokenKind, 78> expected = {
            ZLexerTokenKind::Plus,
            ZLexerTokenKind::Minus,
            ZLexerTokenKind::Asterisk,
            ZLexerTokenKind::Slash,
            ZLexerTokenKind::Percent,
            ZLexerTokenKind::Pipe,
            ZLexerTokenKind::Amp,
            ZLexerTokenKind::Equal,
            ZLexerTokenKind::Exclam,
            ZLexerTokenKind::Tilde,
            ZLexerTokenKind::Caret,

            ZLexerTokenKind::LParen,
            ZLexerTokenKind::RParen,
            ZLexerTokenKind::LBrak,
            ZLexerTokenKind::RBrak,
            ZLexerTokenKind::LBrace,
            ZLexerTokenKind::RBrace,

            ZLexerTokenKind::Dot,
            ZLexerTokenKind::DDot,
            ZLexerTokenKind::Comma,
            ZLexerTokenKind::Colon,
            ZLexerTokenKind::ColonColon,
            ZLexerTokenKind::Semicolon,

            ZLexerTokenKind::PlusPlus,
            ZLexerTokenKind::MinusMinus,
            ZLexerTokenKind::AmpAmp,
            ZLexerTokenKind::AmpOp,
            ZLexerTokenKind::PipePipe,

            ZLexerTokenKind::Lesser,
            ZLexerTokenKind::LesserEqual,
            ZLexerTokenKind::LesserLesser,

            // > >= >>
            ZLexerTokenKind::Greater,
            ZLexerTokenKind::GreaterEqual,
            ZLexerTokenKind::GreaterGreater,

            // > >= >>
            ZLexerTokenKind::Greater,
            ZLexerTokenKind::GreaterEqual,
            ZLexerTokenKind::GreaterGreater,

            ZLexerTokenKind::PlusEqual,
            ZLexerTokenKind::MinusEqual,
            ZLexerTokenKind::AsteriskEqual,
            ZLexerTokenKind::SlashEqual,
            ZLexerTokenKind::PercentEqual,
            ZLexerTokenKind::AmpEqual,
            ZLexerTokenKind::PipeEqual,
            ZLexerTokenKind::EqualEqual,
            ZLexerTokenKind::ExclamEqual,
            ZLexerTokenKind::GreaterGreaterEqual,
            ZLexerTokenKind::LesserLesserEqual,

            ZLexerTokenKind::Arrow,
            ZLexerTokenKind::EqualBig,

            ZLexerTokenKind::ColonEqual,
            ZLexerTokenKind::Qmark,

            ZLexerTokenKind::QAmp,
            ZLexerTokenKind::QPipe,
            ZLexerTokenKind::DQmark,
            ZLexerTokenKind::QExclam,
            ZLexerTokenKind::DExclam,

            ZLexerTokenKind::AtSign,
            ZLexerTokenKind::DAtSign,
            ZLexerTokenKind::AtColon,

            ZLexerTokenKind::True,
            ZLexerTokenKind::False,
            ZLexerTokenKind::Let,
            ZLexerTokenKind::Type,
            ZLexerTokenKind::Return,
            ZLexerTokenKind::Struct,
            ZLexerTokenKind::Enum,
            ZLexerTokenKind::If,
            ZLexerTokenKind::EndIf,
            ZLexerTokenKind::Loop,
            ZLexerTokenKind::EndLoop,
            ZLexerTokenKind::Func,
            ZLexerTokenKind::Vari,
            ZLexerTokenKind::Break,
            ZLexerTokenKind::Continue,
            ZLexerTokenKind::Goto,
            ZLexerTokenKind::Label,

            ZLexerTokenKind::Eof,
        };

        ZLexer lexer(source);

        std::vector<ZLexerTokenType> actual = lexer.finalize();

        // Actual scan size should be the same size of expected tokens size.
        ASSERT_EQ(actual.size(), expected.size());

        // Actual scan should eb the same as expected scan.
        for (std::size_t i = 0; i < expected.size(); ++i) {
            EXPECT_EQ(actual[i].kind, expected[i]) << "Token index: " << i;
        }

        auto& diagnostics = lexer.diagnostics();

        // Expect to have no errors.
        EXPECT_FALSE(diagnostics.has_errors());

        // Expect the concat time to be zero.
        EXPECT_EQ(diagnostics.concat_count(), 0);

        // Expect scan time to be once.
        EXPECT_EQ(diagnostics.scan_count(), 1);
    }

    class ZLexerNumericTest
        : public ::testing::TestWithParam<std::string_view> {};

    /**
     * Expect: Scan hex, octal, binary, decimal, float and scientific numbers.
     * Should:
     *  - Scan numeric numbers.
     */
    TEST_P(ZLexerNumericTest, SingleNumericScan) {
        const std::string_view source = GetParam();

        ZLexerBufferType buffer(source.begin(), source.end());
        ZLexer           lexer(buffer);

        ASSERT_TRUE(lexer.scan());

        const auto tokens = lexer.finalize();

        ASSERT_EQ(tokens.size(), 2);

        EXPECT_EQ(tokens[0].kind, ZLexerTokenKind::Numeric);
        EXPECT_EQ(tokens[1].kind, ZLexerTokenKind::Eof);

        EXPECT_EQ(length<std::size_t>(tokens[0].range), source.size());
    }

    INSTANTIATE_TEST_SUITE_P(NumericLiterals, ZLexerNumericTest,
                             ::testing::Values("0", "1", "42", "123456789",
                                               "0x0", "0x1", "0xFF", "0x123ABC",
                                               "0o0", "0o123", "0o777", "0b0",
                                               "0b1", "0b101010", "0.0", "1.0",
                                               "1.5", "123.456", ".5", ".123",
                                               "1e0", "1e10", "1E10", "1.5e10",
                                               "1.5e-10", "1.5e+10"));

    class ZLexerInvalidNumericTest
        : public ::testing::TestWithParam<std::string_view> {};

    /**
     * Expect: Scan invalid numeric values.
     * Shoud: Report error.
     */
    TEST_P(ZLexerInvalidNumericTest, SingleInvalidNumericScan) {
        const std::string_view source = GetParam();

        ZLexerBufferType buffer(source.begin(), source.end());
        ZLexer           lexer(buffer);

        EXPECT_FALSE(lexer.scan());

        const auto tokens = lexer.finalize();

        EXPECT_TRUE(lexer.diagnostics().has_errors());
    }

    INSTANTIATE_TEST_SUITE_P(InvalidNumericLiterals, ZLexerInvalidNumericTest,
                             ::testing::Values("0x", "0X", "0o", "0O", "0b",
                                               "0B", "1e", "1E", "1e+", "1e-",
                                               "0.", "1.", "0xG", "0x12G",
                                               "0b102", "0o89"));

    /**
     * Expect:
     *  - lexer1 and lexer2 scan independently.
     *  - The split occurs between complete tokens.
     *  - Concatenating them produces the same token stream as the full source.
     */
    TEST(ZLexer, ConcatCompleteBuffers) {
        std::string_view source1 = "let x = 42;";
        std::string_view source2 = "return x + 1;";

        ZLexer lexer1(source1);
        ZLexer lexer2(source2, lexer1.get_end_offset());

        // Both buffers should scan independently without incomplete state.
        ASSERT_TRUE(lexer1.scan());
        ASSERT_TRUE(lexer2.scan());

        // Neither lexer should have errors before concatenation.
        EXPECT_FALSE(lexer1.diagnostics().has_errors());
        EXPECT_FALSE(lexer2.diagnostics().has_errors());

        // Concatenate lexer2 into lexer1.
        lexer1 << lexer2;

        const auto tokens = lexer1.finalize();

        const std::array expected = {
            ZLexerTokenKind::Let,       ZLexerTokenKind::Identifier,
            ZLexerTokenKind::Equal,     ZLexerTokenKind::Numeric,
            ZLexerTokenKind::Semicolon,

            ZLexerTokenKind::Return,    ZLexerTokenKind::Identifier,
            ZLexerTokenKind::Plus,      ZLexerTokenKind::Numeric,
            ZLexerTokenKind::Semicolon,

            ZLexerTokenKind::Eof,
        };

        ASSERT_EQ(tokens.size(), expected.size());

        for (std::size_t i = 0; i < expected.size(); ++i) {
            EXPECT_EQ(tokens[i].kind, expected[i]) << "Token index: " << i;
        }

        auto& diagnostics = lexer1.diagnostics();

        EXPECT_FALSE(diagnostics.has_errors());

        // Since the split is between complete tokens, concat should not need
        // to continue scanning an incomplete token.
        EXPECT_EQ(diagnostics.concat_count(), 1);

        // If there was error in scan of second lexer since we scanned 1 and 2
        // a new scan counts as third scan. but here we have two separate scans.
        EXPECT_EQ(diagnostics.scan_count(), 2);
    }

    /**
     * Expect:
     *  - lexer1, lexer2 and lexer3 scan independently.
     *  - All splits occur between complete tokens.
     *  - Concatenating them produces one correct token stream.
     */
    TEST(ZLexer, ConcatThreeCompleteBuffers) {
        std::string_view source1 = "let x = 42;";
        std::string_view source2 = "return x + 1;";
        std::string_view source3 = "break continue;";

        ZLexer lexer1(source1);
        ZLexer lexer2(source2, lexer1.get_end_offset());
        ZLexer lexer3(source3, lexer2.get_end_offset());

        // All buffers should scan independently.
        ASSERT_TRUE(lexer1.scan());
        ASSERT_TRUE(lexer2.scan());
        ASSERT_TRUE(lexer3.scan());

        // No lexer should have errors.
        EXPECT_FALSE(lexer1.diagnostics().has_errors());
        EXPECT_FALSE(lexer2.diagnostics().has_errors());
        EXPECT_FALSE(lexer3.diagnostics().has_errors());

        // Concatenate all lexers.
        lexer1 << std::move(lexer2) << std::move(lexer3);

        const auto tokens = lexer1.finalize();

        const std::array expected = {
            // lexer1
            ZLexerTokenKind::Let,
            ZLexerTokenKind::Identifier,
            ZLexerTokenKind::Equal,
            ZLexerTokenKind::Numeric,
            ZLexerTokenKind::Semicolon,

            // lexer2
            ZLexerTokenKind::Return,
            ZLexerTokenKind::Identifier,
            ZLexerTokenKind::Plus,
            ZLexerTokenKind::Numeric,
            ZLexerTokenKind::Semicolon,

            // lexer3
            ZLexerTokenKind::Break,
            ZLexerTokenKind::Continue,
            ZLexerTokenKind::Semicolon,

            ZLexerTokenKind::Eof,
        };

        ASSERT_EQ(tokens.size(), expected.size());

        for (std::size_t i = 0; i < expected.size(); ++i) {
            EXPECT_EQ(tokens[i].kind, expected[i]) << "Token index: " << i;
        }

        auto& diagnostics = lexer1.diagnostics();

        EXPECT_FALSE(diagnostics.has_errors());

        // Two concatenation operations.
        EXPECT_EQ(diagnostics.concat_count(), 2);

        EXPECT_EQ(diagnostics.scan_count(), 3);
    }

    /**
     * Expect:
     *  - lexer1 ends in the middle of an identifier.
     *  - lexer2 starts with the remaining identifier characters.
     *  - Concatenating them reconstructs a single Identifier token.
     */
    TEST(ZLexer, ConcatThreeBuffersIdentifierSplit) {
        std::string_view source1 = "let hel";
        std::string_view source2 = "lo = 42;";
        std::string_view source3 = "return hello;";

        ZLexer lexer1(source1);
        ZLexer lexer2(source2, lexer1.get_end_offset());
        ZLexer lexer3(source3, lexer2.get_end_offset());

        // Scan all buffers independently.
        ASSERT_TRUE(lexer1.scan());
        ASSERT_TRUE(lexer2.scan());
        ASSERT_TRUE(lexer3.scan());

        // No lexer should have errors.
        EXPECT_FALSE(lexer1.diagnostics().has_errors());
        EXPECT_FALSE(lexer2.diagnostics().has_errors());
        EXPECT_FALSE(lexer3.diagnostics().has_errors());

        // lexer1 ended in the middle of an identifier.
        // lexer1 + lexer2 should reconstruct: "hello".
        lexer1 << std::move(lexer2) << std::move(lexer3);

        const auto tokens = lexer1.finalize();

        const std::array expected = {
            ZLexerTokenKind::Let,
            ZLexerTokenKind::Identifier,  // hello
            ZLexerTokenKind::Equal,      ZLexerTokenKind::Numeric,
            ZLexerTokenKind::Semicolon,

            ZLexerTokenKind::Return,
            ZLexerTokenKind::Identifier,  // hello
            ZLexerTokenKind::Semicolon,

            ZLexerTokenKind::Eof,
        };

        ASSERT_EQ(tokens.size(), expected.size());

        for (std::size_t i = 0; i < expected.size(); ++i) {
            EXPECT_EQ(tokens[i].kind, expected[i]) << "Token index: " << i;
        }

        auto& diagnostics = lexer1.diagnostics();

        EXPECT_FALSE(diagnostics.has_errors());

        // Two concatenation operations.
        EXPECT_EQ(diagnostics.concat_count(), 2);

        // Three lexer scans.
        EXPECT_EQ(diagnostics.scan_count(), 3);
    }

    /**
     * Expect:
     *  - lexer1 ends in the middle of a numeric literal.
     *  - lexer2 starts with the remaining numeric characters.
     *  - Concatenating them reconstructs a single Numeric token.
     */
    TEST(ZLexer, ConcatThreeBuffersNumberSplit) {
        std::string_view source1 = "let x = 12";
        std::string_view source2 = "34 + 1;";
        std::string_view source3 = "return 5678;";

        ZLexer lexer1(source1);
        ZLexer lexer2(source2, lexer1.get_end_offset());
        ZLexer lexer3(source3, lexer2.get_end_offset());

        // Scan all buffers independently.
        ASSERT_TRUE(lexer1.scan());
        ASSERT_TRUE(lexer2.scan());
        ASSERT_TRUE(lexer3.scan());

        // No lexer should have errors.
        EXPECT_FALSE(lexer1.diagnostics().has_errors());
        EXPECT_FALSE(lexer2.diagnostics().has_errors());
        EXPECT_FALSE(lexer3.diagnostics().has_errors());

        // lexer1 ends in the middle of the numeric literal "1234".
        lexer1 << std::move(lexer2) << std::move(lexer3);

        const auto tokens = lexer1.finalize();

        const std::array expected = {
            // "let x = 1234 + 1;"
            ZLexerTokenKind::Let,
            ZLexerTokenKind::Identifier,
            ZLexerTokenKind::Equal,
            ZLexerTokenKind::Numeric,
            ZLexerTokenKind::Plus,
            ZLexerTokenKind::Numeric,
            ZLexerTokenKind::Semicolon,

            // "return 5678;"
            ZLexerTokenKind::Return,
            ZLexerTokenKind::Numeric,
            ZLexerTokenKind::Semicolon,

            ZLexerTokenKind::Eof,
        };

        ASSERT_EQ(tokens.size(), expected.size());

        for (std::size_t i = 0; i < expected.size(); ++i) {
            EXPECT_EQ(tokens[i].kind, expected[i]) << "Token index: " << i;
        }

        auto& diagnostics = lexer1.diagnostics();

        EXPECT_FALSE(diagnostics.has_errors());

        // Two concatenation operations.
        EXPECT_EQ(diagnostics.concat_count(), 2);

        // Three lexer scans.
        EXPECT_EQ(diagnostics.scan_count(), 3);
    }

    TEST(ZLexer, StartAndEndPositions) {
        std::string_view source1 = "let x = 12";
        std::string_view source2 = "let x = 12;";
        std::string_view source3 = "let x = 12;";

        ZLexer lexer1(source1);
        ZLexer lexer2(source2, lexer1.get_end_offset());
        ZLexer lexer3(source3, lexer2.get_end_offset());

        // Scan all buffers independently.
        ASSERT_TRUE(lexer1.scan());
        ASSERT_TRUE(lexer2.scan());
        ASSERT_TRUE(lexer3.scan());

        ASSERT_TRUE(lexer1.get_end_offset() == lexer2.get_start_offset());
    }

    TEST(ZLexer, HashBeforeIf) {
        std::string_view source1 = "#if";
        std::string_view source2 = "x";

        ZLexer lexer1(source1);
        ZLexer lexer2(source2, lexer1.get_end_offset());

        ASSERT_TRUE(lexer1.scan());
        ASSERT_TRUE(lexer2.scan());
        EXPECT_FALSE(lexer1.diagnostics().has_errors());
        EXPECT_FALSE(lexer2.diagnostics().has_errors());

        lexer1 << std::move(lexer2);
        const auto tokens = lexer1.finalize();

        // 4 becaus it has one EOF at the end
        ASSERT_EQ(tokens.size(), 4);
        ASSERT_EQ(tokens[0].kind, ZLexerTokenKind::Hash);
        ASSERT_EQ(tokens[1].kind, ZLexerTokenKind::If);
        ASSERT_EQ(tokens[2].kind, ZLexerTokenKind::Identifier);
    }

    TEST(ZLexer, TokenFlagsSingleBuffer) {
        std::string_view source = "x\n#if y\n  z";

        ZLexer lexer(source);
        ASSERT_TRUE(lexer.scan());

        const auto tokens = lexer.finalize();
        ASSERT_EQ(tokens.size(), 6);

        EXPECT_EQ(tokens[0].kind, ZLexerTokenKind::Identifier);
        EXPECT_TRUE(flagged(tokens[0], TokenFlags::AtLineStart));
        EXPECT_FALSE(flagged(tokens[0], TokenFlags::WhiteSpaceBefore));

        EXPECT_EQ(tokens[1].kind, ZLexerTokenKind::Hash);
        EXPECT_EQ(tokens[1].flags, to_underlying(TokenFlags::AtLineStart |
                                                 TokenFlags::WhiteSpaceBefore));

        EXPECT_EQ(tokens[2].kind, ZLexerTokenKind::If);
        EXPECT_EQ(tokens[2].flags, 0);

        EXPECT_EQ(tokens[3].kind, ZLexerTokenKind::Identifier);
        EXPECT_FALSE(flagged(tokens[3], TokenFlags::AtLineStart));
        EXPECT_TRUE(flagged(tokens[3], TokenFlags::WhiteSpaceBefore));

        EXPECT_EQ(tokens[4].kind, ZLexerTokenKind::Identifier);
        EXPECT_TRUE(flagged(tokens[4], TokenFlags::AtLineStart));
    }

    /**
     * Expect: every token's range is half-open and spells the token.
     * Should: hold for keywords, identifiers, numerics and single-char
     * punctuation alike.
     * TODO: Punctuation -> emitting it as
     * (start, start) leaves a zero-length range that silently spells "".
     */
    TEST(ZLexer, TokenRangesAreHalfOpen) {
        std::string_view source = "let x = 42;";

        ZLexer lexer(source);
        ASSERT_TRUE(lexer.scan());

        const auto tokens = lexer.finalize();
        ASSERT_EQ(tokens.size(), 6);

        expect_token(source, tokens[0], ZLexerTokenKind::Let, "let");
        expect_token(source, tokens[1], ZLexerTokenKind::Identifier, "x");
        expect_token(source, tokens[2], ZLexerTokenKind::Equal, "=");
        expect_token(source, tokens[3], ZLexerTokenKind::Numeric, "42");
        expect_token(source, tokens[4], ZLexerTokenKind::Semicolon, ";");

        // Eof is the one legitimately empty range.
        EXPECT_EQ(tokens[5].kind, ZLexerTokenKind::Eof);
        EXPECT_EQ(tokens[5].range.begin, tokens[5].range.end);
    }

    /**
     * Expect: no token but Eof is zero-length.
     * Should: a range that spells nothing means every consumer that reads a
     * token by its range sees an empty string.
     */
    TEST(ZLexer, OnlyEofHasAnEmptyRange) {
        std::string_view source = "a + b; (c) [d] {e} x.y";

        ZLexer lexer(source);
        ASSERT_TRUE(lexer.scan());

        for (const auto& t: lexer.finalize()) {
            if (t.kind == ZLexerTokenKind::Eof) continue;

            EXPECT_LT(t.range.begin, t.range.end)
                << "zero-length range for " << to_string(t.kind);
        }
    }

    /**
     * Expect: a quoted literal becomes one String token.
     * Should: the range covers the quotes, so the spelling round-trips. Before
     * this was pinned the quotes were consumed silently and the payload lexed
     * as ordinary identifiers, which no kind-only assertion noticed.
     */
    TEST(ZLexer, StringLiteralsProduceStringTokens) {
        std::string_view source = "\"ab\" '' \"\" \"a\\\"b\"";

        ZLexer lexer(source);
        ASSERT_TRUE(lexer.scan());

        const auto tokens = lexer.finalize();
        ASSERT_EQ(tokens.size(), 5);

        expect_token(source, tokens[0], ZLexerTokenKind::String, "\"ab\"");
        expect_token(source, tokens[1], ZLexerTokenKind::String, "''");
        expect_token(source, tokens[2], ZLexerTokenKind::String, "\"\"");

        // The escaped quote does not end the literal.
        expect_token(source, tokens[3], ZLexerTokenKind::String, "\"a\\\"b\"");

        EXPECT_EQ(tokens[4].kind, ZLexerTokenKind::Eof);
    }

    /**
     * Expect: a string keeps its payload out of the token stream.
     * Should: characters that would otherwise lex as operators stay inside the
     * one String token.
     */
    TEST(ZLexer, StringPayloadIsNotLexed) {
        std::string_view source = "\"a + b == c\"";

        ZLexer lexer(source);
        ASSERT_TRUE(lexer.scan());

        const auto tokens = lexer.finalize();
        ASSERT_EQ(tokens.size(), 2);

        expect_token(source, tokens[0], ZLexerTokenKind::String,
                     "\"a + b == c\"");
        EXPECT_EQ(tokens[1].kind, ZLexerTokenKind::Eof);
    }

    /**
     * Expect: a merged multi-char operator spans both of its characters.
     * Should: merging two single-char tokens has to widen the range, not just
     * rewrite the kind.
     */
    TEST(ZLexer, MergedOperatorRangesSpanBothCharacters) {
        std::string_view source = "a == b != c";

        ZLexer lexer(source);
        ASSERT_TRUE(lexer.scan());

        const auto tokens = lexer.finalize();
        ASSERT_EQ(tokens.size(), 6);

        expect_token(source, tokens[0], ZLexerTokenKind::Identifier, "a");
        expect_token(source, tokens[1], ZLexerTokenKind::EqualEqual, "==");
        expect_token(source, tokens[2], ZLexerTokenKind::Identifier, "b");
        expect_token(source, tokens[3], ZLexerTokenKind::ExclamEqual, "!=");
        expect_token(source, tokens[4], ZLexerTokenKind::Identifier, "c");
    }

    /**
     * Expect: consecutive buffers are offset-contiguous.
     * Should: the second buffer's first character sits exactly at the first
     * buffer's end offset. A gap there makes every range in the second buffer
     * index the concatenated source one character off.
     */
    TEST(ZLexer, ConcatenatedBuffersAreOffsetContiguous) {
        std::string_view source1 = "let hel";
        std::string_view source2 = "lo = 42;";

        // What the two buffers spell once joined.
        const std::string whole = std::string(source1) + std::string(source2);

        ZLexer lexer1(source1);
        ZLexer lexer2(source2, lexer1.get_end_offset());

        EXPECT_EQ(lexer1.get_start_offset(), 0u);
        EXPECT_EQ(lexer1.get_end_offset(), source1.size());
        EXPECT_EQ(lexer2.get_start_offset(), lexer1.get_end_offset());
        EXPECT_EQ(lexer2.get_end_offset(), whole.size());

        ASSERT_TRUE(lexer1.scan());
        ASSERT_TRUE(lexer2.scan());

        lexer1 << std::move(lexer2);
        const auto tokens = lexer1.finalize();

        ASSERT_EQ(tokens.size(), 6);

        // Ranges index the joined source, and the split identifier is one
        // token spanning the seam.
        expect_token(whole, tokens[0], ZLexerTokenKind::Let, "let");
        expect_token(whole, tokens[1], ZLexerTokenKind::Identifier, "hello");
        expect_token(whole, tokens[2], ZLexerTokenKind::Equal, "=");
        expect_token(whole, tokens[3], ZLexerTokenKind::Numeric, "42");
        expect_token(whole, tokens[4], ZLexerTokenKind::Semicolon, ";");
    }

    /**
     * Expect: a string literal split across two buffers reconstructs.
     * Should: the String carries the offset of its opening quote from the
     * previous buffer, and everything after the seam keeps its true position.
     */
    TEST(ZLexer, ConcatStringSplitAcrossBuffers) {
        std::string_view source1 = "let s = \"ab";
        std::string_view source2 = "cd\";";

        const std::string whole = std::string(source1) + std::string(source2);

        ZLexer lexer1(source1);
        ZLexer lexer2(source2, lexer1.get_end_offset());

        // The first buffer ends inside the literal, so it reports that more
        // input is needed and stays in the string state.
        EXPECT_FALSE(lexer1.scan());
        EXPECT_NE(lexer1.get_state(), ZLexerInternalState::Normal);

        // Scanned on its own the tail is meaningless -- it opens a string it
        // never closes. concat discards this result and rescans in context.
        lexer2.scan();

        lexer1 << std::move(lexer2);
        const auto tokens = lexer1.finalize();

        ASSERT_EQ(tokens.size(), 6);

        expect_token(whole, tokens[0], ZLexerTokenKind::Let, "let");
        expect_token(whole, tokens[1], ZLexerTokenKind::Identifier, "s");
        expect_token(whole, tokens[2], ZLexerTokenKind::Equal, "=");
        expect_token(whole, tokens[3], ZLexerTokenKind::String, "\"abcd\"");
        expect_token(whole, tokens[4], ZLexerTokenKind::Semicolon, ";");

        EXPECT_EQ(tokens[5].kind, ZLexerTokenKind::Eof);
        EXPECT_EQ(tokens[5].range.begin, whole.size());
    }

    /**
     * Expect: a literal spanning three buffers still reconstructs.
     * Should: the token start survives every seam, not just the first. Each
     * concat leaves _buffer untouched, so resuming from the buffer's end
     * rather than from the scan position loses one buffer's worth of offset
     * per extra seam.
     */
    TEST(ZLexer, ConcatStringSplitAcrossThreeBuffers) {
        std::string_view source1 = "let s = \"ab";
        std::string_view source2 = "cd";
        std::string_view source3 = "ef\";";

        const std::string whole =
            std::string(source1) + std::string(source2) + std::string(source3);

        ZLexer lexer1(source1);
        ZLexer lexer2(source2, lexer1.get_end_offset());
        ZLexer lexer3(source3, lexer2.get_end_offset());

        lexer1.scan();
        lexer2.scan();
        lexer3.scan();

        lexer1 << std::move(lexer2) << std::move(lexer3);
        const auto tokens = lexer1.finalize();

        ASSERT_EQ(tokens.size(), 6);

        expect_token(whole, tokens[0], ZLexerTokenKind::Let, "let");
        expect_token(whole, tokens[1], ZLexerTokenKind::Identifier, "s");
        expect_token(whole, tokens[2], ZLexerTokenKind::Equal, "=");
        expect_token(whole, tokens[3], ZLexerTokenKind::String, "\"abcdef\"");
        expect_token(whole, tokens[4], ZLexerTokenKind::Semicolon, ";");

        EXPECT_EQ(tokens[5].kind, ZLexerTokenKind::Eof);
        EXPECT_EQ(tokens[5].range.begin, whole.size());
    }

    /**
     * Expect: a block comment spanning three buffers stays trivia.
     * Should: it produces no token, and the code after it keeps its true
     * offsets. Comments resume through the same path as literals.
     */
    TEST(ZLexer, ConcatBlockCommentSplitAcrossBuffers) {
        std::string_view source1 = "let /* a ";
        std::string_view source2 = " b ";
        std::string_view source3 = " c */ x;";

        const std::string whole =
            std::string(source1) + std::string(source2) + std::string(source3);

        ZLexer lexer1(source1);
        ZLexer lexer2(source2, lexer1.get_end_offset());
        ZLexer lexer3(source3, lexer2.get_end_offset());

        lexer1.scan();
        lexer2.scan();
        lexer3.scan();

        lexer1 << std::move(lexer2) << std::move(lexer3);
        const auto tokens = lexer1.finalize();

        ASSERT_EQ(tokens.size(), 4);

        expect_token(whole, tokens[0], ZLexerTokenKind::Let, "let");
        expect_token(whole, tokens[1], ZLexerTokenKind::Identifier, "x");
        expect_token(whole, tokens[2], ZLexerTokenKind::Semicolon, ";");

        EXPECT_EQ(tokens[3].kind, ZLexerTokenKind::Eof);
    }
}  // namespace Z::Zaban::Tests
