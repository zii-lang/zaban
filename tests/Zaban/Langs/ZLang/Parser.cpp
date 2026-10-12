#include <gtest/gtest.h>

#include <Z/Zaban/Langs/ZLang/Lexer.hpp>
#include <Z/Zaban/Langs/ZLang/Parser.hpp>

namespace Z::Zaban::Tests {
    using namespace Z::Zaban::Langs::ZLang;

    TEST(ZParser, ParseEmptySource) {
        std::string_view source = {""};

        ZLexer                       lexer(source);
        std::vector<ZLexerTokenType> tokens;
        EXPECT_NO_FATAL_FAILURE(tokens = lexer.finalize());

        EXPECT_EQ(1, tokens.size());
        ASSERT_FALSE(tokens.empty());

        const auto& token = tokens.back();
        EXPECT_EQ(ZLexerTokenKind::Eof, token.kind);

        ZTokenStream token_stream(tokens);
        ZParser      parser(token_stream);
        auto         module = parser.parse();

        ASSERT_TRUE(module.empty());
    }
}  // namespace Z::Zaban::Tests
