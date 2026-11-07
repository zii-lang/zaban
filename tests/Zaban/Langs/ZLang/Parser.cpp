#include <gtest/gtest.h>

#include <Z/Zaban/AST/Atomics/Literal.hpp>
#include <Z/Zaban/AST/Expressions/PrimaryExpression.hpp>
#include <Z/Zaban/AST/Module.hpp>
#include <Z/Zaban/AST/Node.hpp>
#include <Z/Zaban/AST/Statement.hpp>
#include <Z/Zaban/AST/Statements/ExpressionStatement.hpp>
#include <Z/Zaban/Langs/ZLang/Lexer.hpp>
#include <Z/Zaban/Langs/ZLang/LexerDiagnostic.hpp>
#include <Z/Zaban/Langs/ZLang/Parser.hpp>
#include <Z/Zaban/Langs/ZLang/Token.hpp>
#include <Z/Zaban/Parse/TokenStream.hpp>

namespace Z::Zaban::Tests {
    using namespace Z::Zaban::Langs::ZLang;

    struct Harness {
        std::string_view               m_source;
        AST::Module<ZOffsetType>       m_module;
        const ZLexerDiagnosticContext* m_lex_diag;

        explicit Harness(std::string_view src) : m_source(src) {
            ZLexer lexer(this->m_source);
            lexer.scan();
            auto tokens = lexer.finalize();
            this->m_lex_diag =
                lexer.diagnostics().cast<ZLexerDiagnosticContext>();
            // TODO: maybe preprocessor.
            Parse::TokenStream<ZTokenKind, ZOffsetType> token_stream(tokens);
            Langs::ZLang::ZParser                       parser(token_stream);
            this->m_module = parser.parse();
        }
    };

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

        Parse::TokenStream<ZTokenKind, ZOffsetType> numeric_source_tokens(
            {ZTokenFactory::gen(ZTokenKind::Numeric, 0, 0),
             ZTokenFactory::gen(ZTokenKind::Semicolon, 0, 0),
             ZTokenFactory::gen(ZTokenKind::Eof, 0, 0)});
        parser = ZParser(numeric_source_tokens);
        module = parser.parse();
        ASSERT_EQ(module.size(), 1);
        ASSERT_EQ(module[0]->node_kind(), AST::NodeKind::Statement);

        AST::Statement<ZOffsetType> numeric_statement_expr =
            module[0]->cast<AST::StatementNode<ZOffsetType>>();
        literal = get_literal(numeric_statement_expr);
        ASSERT_EQ(literal->get_kind(), AST::Atomics::LiteralKind::Numeric);
    };

    TEST(ZParser, ParseLiteralsWithSource) {
        Harness h{"null;"};
        auto    module = h.m_module;

        ASSERT_EQ(module.size(), 1);
        AST::Statement<ZOffsetType> null_st_expr =
            module[0]->cast<AST::StatementNode<ZOffsetType>>();

        auto literal = get_literal(null_st_expr);
        ASSERT_EQ(literal->get_kind(), AST::Atomics::LiteralKind::Null);
    }

    TEST(ZParser, ParseUnaryExpressionStatement) {
        Harness h{"1++;"};
        auto    module = h.m_module;

        ASSERT_EQ(module.size(), 1);
        auto statement = module[0]->cast<AST::StatementNode<ZOffsetType>>();
        ASSERT_EQ(statement->statement_kind(), AST::StatementKind::Expression);
        auto expr_statement =
            statement
                ->cast<AST::Statements::ExpressionStatement<ZOffsetType>>();
        auto expr = expr_statement->get();
        ASSERT_EQ(expr->expr_kind(), AST::ExpressionKind::Suffix);

        Harness h2{"++1;"};
        module = h2.m_module;

        ASSERT_EQ(module.size(), 1);
        statement = module[0]->cast<AST::StatementNode<ZOffsetType>>();
        ASSERT_EQ(statement->statement_kind(), AST::StatementKind::Expression);
        expr_statement =
            statement
                ->cast<AST::Statements::ExpressionStatement<ZOffsetType>>();
        expr = expr_statement->get();
        ASSERT_EQ(expr->expr_kind(), AST::ExpressionKind::Prefix);
    }
}  // namespace Z::Zaban::Tests
