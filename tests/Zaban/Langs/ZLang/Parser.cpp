#include <gtest/gtest.h>

#include <Z/Zaban/Langs/ZLang/Parser.hpp>
#include <memory>

namespace Z::Zaban::Tests {
    using namespace Z::Zaban::Langs::ZLang;

    TEST(ZParser, ParseEmptyTokenStream) {
        Z::Zaban::Parse::TokenStream<ZTokenKind, ZOffsetType> token_stream{};
        Z::Zaban::Langs::ZLang::ZParser parser(token_stream);
        Z::Zaban::AST::Module           module = parser.parse();

        ASSERT_EQ(module.size(), 0);
    }

}  // namespace Z::Zaban::Tests
