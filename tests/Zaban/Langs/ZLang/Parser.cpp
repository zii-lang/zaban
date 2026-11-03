#include <gtest/gtest.h>

#include <Z/Zaban/AST/Node.hpp>
#include <Z/Zaban/AST/Statement.hpp>
#include <Z/Zaban/AST/Statements/ExpressionStatement.hpp>
#include <Z/Zaban/Langs/ZLang/Parser.hpp>
#include <Z/Zaban/Langs/ZLang/Token.hpp>
#include <Z/Zaban/Parse/TokenStream.hpp>

#include "Z/Zaban/AST/Atomics/Literal.hpp"
#include "Z/Zaban/AST/Expressions/PrimaryExpression.hpp"

namespace Z::Zaban::Tests {
    using namespace Z::Zaban::Langs::ZLang;

    TEST(ZParser, ParseEmptyTokenStream) {
        Parse::TokenStream<ZTokenKind, ZOffsetType> token_stream{};
        Langs::ZLang::ZParser                       parser(token_stream);
        AST::Module                                 module = parser.parse();

        ASSERT_EQ(module.size(), 0);
    }

    template<typename OffsetType>
    auto get_literal(const AST::Statement<OffsetType>& statement) {
        EXPECT_EQ(statement->statement_kind(), AST::StatementKind::Expression);

        auto expr_statement = statement->template cast<
            AST::Statements::ExpressionStatement<OffsetType>>();

        auto expression = expr_statement->get();

        auto primary = expression->template cast<
            AST::Expressions::PrimaryExpression<OffsetType>>();

        auto atomic = primary->get_value();

        return atomic->template cast<AST::Atomics::Literal<OffsetType>>();
    }

    TEST(ZParser, ParseLiterals) {
        Parse::TokenStream<ZTokenKind, ZOffsetType> null_source_tokens({
            ZTokenFactory::gen(ZTokenKind::Null, 0, 4),
            ZTokenFactory::gen(ZTokenKind::Semicolon, 5, 6),
            ZTokenFactory::gen(ZTokenKind::Eof, 0, 0),
        });

        Langs::ZLang::ZParser parser(null_source_tokens);
        AST::Module           module = parser.parse();

        ASSERT_EQ(module.size(), 1);
        ASSERT_EQ(module[0]->node_kind(), AST::NodeKind::Statement);
        AST::Statement<ZOffsetType> null_statement_expr =
            module[0]->cast<AST::StatementNode<ZOffsetType>>();
        auto literal = get_literal<ZOffsetType>(null_statement_expr);
        ASSERT_EQ(literal->get_kind(), AST::Atomics::LiteralKind::Null);

        Parse::TokenStream<ZTokenKind, ZOffsetType> true_source_tokens(
            {ZTokenFactory::gen(ZTokenKind::True, 0, 0),
             ZTokenFactory::gen(ZTokenKind::Semicolon, 0, 0),
             ZTokenFactory::gen(ZTokenKind::Eof, 0, 0)});
        parser = ZParser(true_source_tokens);
        module = parser.parse();
        ASSERT_EQ(module.size(), 1);
        ASSERT_EQ(module[0]->node_kind(), AST::NodeKind::Statement);

        AST::Statement<ZOffsetType> true_statement_expr =
            module[0]->cast<AST::StatementNode<ZOffsetType>>();
        literal = get_literal(true_statement_expr);
        ASSERT_EQ(literal->get_kind(), AST::Atomics::LiteralKind::Boolean);
        ASSERT_EQ(literal->get_bool(), true);


        Parse::TokenStream<ZTokenKind, ZOffsetType> false_source_tokens(
            {ZTokenFactory::gen(ZTokenKind::False, 0, 0),
             ZTokenFactory::gen(ZTokenKind::Semicolon, 0, 0),
             ZTokenFactory::gen(ZTokenKind::Eof, 0, 0)});
        parser = ZParser(false_source_tokens);
        module = parser.parse();
        ASSERT_EQ(module.size(), 1);
        ASSERT_EQ(module[0]->node_kind(), AST::NodeKind::Statement);

        AST::Statement<ZOffsetType> false_statement_expr =
            module[0]->cast<AST::StatementNode<ZOffsetType>>();
        literal = get_literal(false_statement_expr);
        ASSERT_EQ(literal->get_kind(), AST::Atomics::LiteralKind::Boolean);
        ASSERT_EQ(literal->get_bool(), false);
    };

}  // namespace Z::Zaban::Tests
