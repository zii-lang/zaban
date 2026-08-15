#include <gtest/gtest.h>

#include <Z/Zaban/Langs/ZLang/Lexer.hpp>

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
}  // namespace Z::Zaban::Tests
