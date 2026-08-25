#include <gtest/gtest.h>

#include <Z/Zaban/Langs/ZLang/Lexer.hpp>
#include <array>

namespace Z::Zaban::Tests {
    using namespace Z::Zaban::Langs::ZLang;

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

        // auto& diagnostics = lexer.diagnostics();

        // Expect to have no errors.
        // EXPECT_FALSE(diagnostics.has_errors());

        // Expect the concat time to be zero.
        // EXPECT_EQ(diagnostics.get_concat_count(), 0);

        // Expect scan time to be once.
        // EXPECT_EQ(diagnostics.get_scan_count(), 1);
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

    TEST_P(ZLexerInvalidNumericTest, SingleInvalidNumericScan) {
        const std::string_view source = GetParam();

        ZLexerBufferType buffer(source.begin(), source.end());
        ZLexer           lexer(buffer);

        EXPECT_FALSE(lexer.scan());

        const auto tokens = lexer.finalize();

        // EXPECT_TRUE(tokens.empty());
        // EXPECT_TRUE(lexer.diagnostic_ctx().has_errors());

        // auto x =
        // static_cast<ZLexerDiagnosticContext&>(lexer.diagnostic_ctx()); for
        // (auto e: x.all()) {
        //     std::cout << static_cast<int>(e.kind()) << std::endl;
        // }
        // EXPECT_TRUE(lexer.diagnostics().has_errors());
    }

    INSTANTIATE_TEST_SUITE_P(InvalidNumericLiterals, ZLexerInvalidNumericTest,
                             ::testing::Values("0x", "0X", "0o", "0O", "0b",
                                               "0B", "1e", "1E", "1e+", "1e-",
                                               "0.", "1.", "0xG", "0x12G",
                                               "0b102", "0o89"));

}  // namespace Z::Zaban::Tests
