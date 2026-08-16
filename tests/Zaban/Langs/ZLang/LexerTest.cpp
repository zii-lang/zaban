#include <gtest/gtest.h>

#include <Z/Zaban/Langs/ZLang/Lexer.hpp>
#include <array>

namespace Z::Zaban::Tests {
    using namespace Z::Zaban::Langs::ZLang;

    /**
     * Expect: finalize lexer scan.
     * Should: not fail, there are no tokens only EOF.
     */
    TEST(LexerTest, ScanSingleLexerEmptySource) {
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
    TEST(LexerTest, ScanSingleLexerAllTokens) {
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
    }
}  // namespace Z::Zaban::Tests
