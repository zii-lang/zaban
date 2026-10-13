
#include <Z/Zaban/AST/Statements/BlockStatement.hpp>
#include <Z/Zaban/AST/Statements/DeclarationStatement.hpp>
#include <Z/Zaban/AST/Statements/ExpressionStatement.hpp>
#include <Z/Zaban/AST/Statements/FlowStatement.hpp>
#include <Z/Zaban/Langs/CLang/AST/Statements/DoWhileStatement.hpp>
#include <Z/Zaban/Langs/CLang/AST/Statements/ForStatement.hpp>
#include <Z/Zaban/Langs/CLang/AST/Statements/IfStatement.hpp>
#include <Z/Zaban/Langs/CLang/AST/Statements/WhileStatement.hpp>
#include <Z/Zaban/Langs/CLang/Parser.hpp>
#include <Z/Zaban/Langs/CLang/ParserDiagnostic.hpp>

namespace Z::Zaban::Langs::CLang {
    using ExpressionStatement =
        Zaban::AST::Statements::ExpressionStatement<COffsetType>;
    using BlockStatement = Zaban::AST::Statements::BlockStatement<COffsetType>;
    using DeclarationStatement =
        Zaban::AST::Statements::DeclarationStatement<COffsetType>;
    using FlowStatement = Zaban::AST::Statements::FlowStatement<COffsetType>;
    using DoWhileStatement =
        Zaban::Langs::CLang::AST::Statements::DoWhileStatementNode<COffsetType>;
    using IfStatement =
        Zaban::Langs::CLang::AST::Statements::IfStatementNode<COffsetType>;
    using ForStatement =
        Zaban::Langs::CLang::AST::Statements::ForStatementNode<COffsetType>;
    using WhileStatement =
        Zaban::Langs::CLang::AST::Statements::WhileStatementNode<COffsetType>;

    CStatement CParser::parse_statement() const {
        auto current = this->peek();
        if (!current) {
            this->report(CParserDiagnosticKind::ErrorUnexpectedEndOfFile);
            return nullptr;
        }

        switch (current->kind) {
            case CTokenKind::Return: {
                return this->parse_return_statement();
            }
            case CTokenKind::Break: {
                return this->parse_break_statement();
            }
            case CTokenKind::Continue: {
                return this->parse_continue_statement();
            }
            case CTokenKind::If: {
                return this->parse_if_statement();
            }
            case CTokenKind::While: {
                return this->parse_while_statement();
            }
            case CTokenKind::Do: {
                return this->parse_do_while_statement();
            }
            case CTokenKind::For: {
                return this->parse_for_statement();
            }
            case CTokenKind::LBrace: {
                return this->parse_block_statement();
            }
            case CTokenKind::Semicolon: {
                this->advance();
                return make_node<BlockStatement>(current->range,
                                                 std::vector<CStatement>{});
            }
            default:
                break;
        }
        // its a declaration if true
        if (is_type_specifier(current->kind)) {
            // TODO: return the result in a DeclarationStatement
        }

        return this->parse_expression_statement();
    }

    // { statement* }
    CStatement CParser::parse_block_statement() const {
        auto lbrace = this->expect(CTokenKind::LBrace,
                                   CParserDiagnosticKind::ErrorExpectedToken);
        if (!lbrace) {
            return nullptr;
        }
        std::vector<CStatement> inner{};
        while (!this->check(CTokenKind::RBrace) && !this->m_stream.end()) {
            auto stmt = this->parse_statement();
            if (stmt) {
                inner.push_back(std::move(stmt));
            } else {
                if (this->check(CTokenKind::RBrace)) {
                    break;
                }
                this->synchronize();
            }
        }
        auto rbrace = this->expect(CTokenKind::RBrace,
                                   CParserDiagnosticKind::ErrorExpectedToken);
        if (!rbrace) return nullptr;
        return make_node<BlockStatement>(span(lbrace->range, rbrace->range),
                                         std::move(inner));
    }

    // if ( expr ) statement [ else statement ]
    CStatement CParser::parse_if_statement() const {
        auto if_tok = this->expect(CTokenKind::If,
                                   CParserDiagnosticKind::ErrorExpectedToken);

        if (!this->expect(CTokenKind::LParen,
                          CParserDiagnosticKind::ErrorExpectedToken)) {
            return nullptr;
        }
        auto condition = this->parse_expression();
        if (!condition) return nullptr;
        if (!this->expect(CTokenKind::RParen,
                          CParserDiagnosticKind::ErrorExpectedToken)) {
            return nullptr;
        }
        auto then_branch = this->parse_statement();
        if (!then_branch) return nullptr;

        if (this->check(CTokenKind::Else)) {
            this->advance();
            auto else_branch = this->parse_statement();
            if (!else_branch) return nullptr;

            auto range = span(if_tok->range, else_branch->location());
            return make_node<IfStatement>(range, std::move(condition),
                                          std::move(then_branch),
                                          std::move(else_branch));
        }

        auto range = span(if_tok->range, then_branch->location());
        return make_node<IfStatement>(range, std::move(condition),
                                      std::move(then_branch));
        //
        // TODO: once declarations exist, a declaration directly as a branch
        // (if (x) int y;) is invalid C. report ErrorDeclarationNotAllowed
        // but still return the node. the syntax was fine, so don't make the
        // caller recover.
    }

    // while ( expr ) statement
    CStatement CParser::parse_while_statement() const {
        auto start_tok = this->expect(
            CTokenKind::While, CParserDiagnosticKind::ErrorExpectedToken);

        if (!this->expect(CTokenKind::LParen,
                          CParserDiagnosticKind::ErrorExpectedToken)) {
            return nullptr;
        }
        auto condition = this->parse_expression();
        if (!condition) return nullptr;
        if (!this->expect(CTokenKind::RParen,
                          CParserDiagnosticKind::ErrorExpectedToken)) {
            return nullptr;
        }
        ++m_loop_depth;
        auto body = this->parse_statement();
        // returning early first would leave the depth one too high and
        // every later break/continue outside a loop would be accepted.
        --m_loop_depth;
        if (!body) return nullptr;
        auto range = span(start_tok->range, body->location());
        return make_node<WhileStatement>(range, std::move(condition),
                                         std::move(body));
    }

    // do statement while ( expr ) ;
    CStatement CParser::parse_do_while_statement() const {
        auto start_tok = this->expect(
            CTokenKind::Do, CParserDiagnosticKind::ErrorExpectedToken);
        ++m_loop_depth;
        auto body = this->parse_statement();
        --m_loop_depth;
        if (!body) return nullptr;

        if (!this->expect(CTokenKind::While,
                          CParserDiagnosticKind::ErrorExpectedToken)) {
            return nullptr;
        }
        if (!this->expect(CTokenKind::LParen,
                          CParserDiagnosticKind::ErrorExpectedToken)) {
            return nullptr;
        }
        auto condition = this->parse_expression();
        if (!condition) return nullptr;
        if (!this->expect(CTokenKind::RParen,
                          CParserDiagnosticKind::ErrorExpectedToken)) {
            return nullptr;
        }
        auto semi = this->expect(CTokenKind::Semicolon,
                                 CParserDiagnosticKind::ErrorExpectedToken);
        if (!semi) return nullptr;
        auto range = span(start_tok->range, semi->range);
        return make_node<DoWhileStatement>(range, std::move(body),
                                           std::move(condition));
    }

    // for ( init ; condition ; step ) statement
    // every clause is optional and is passed as nullptr when missing.
    CStatement CParser::parse_for_statement() const {
        auto start_tok = this->expect(
            CTokenKind::For, CParserDiagnosticKind::ErrorExpectedToken);
        if (!this->expect(CTokenKind::LParen,
                          CParserDiagnosticKind::ErrorExpectedToken)) {
            return nullptr;
        }
        CStatement init = nullptr;
        if (this->check(CTokenKind::Semicolon)) {
            this->advance();
        } else if (const auto* tok = this->peek();
                   tok && this->is_type_specifier(tok->kind)) {
            auto decl = this->parse_declaration();
            if (!decl) return nullptr;
            const auto range = decl->location();
            init = make_node<DeclarationStatement>(range, std::move(decl));
        } else {
            init = this->parse_expression_statement();
            if (!init) return nullptr;
        }

        CExpression condition = nullptr;
        if (!this->check(CTokenKind::Semicolon)) {
            condition = this->parse_expression();
            if (!condition) return nullptr;
        }
        if (!this->expect(CTokenKind::Semicolon,
                          CParserDiagnosticKind::ErrorExpectedToken)) {
            return nullptr;
        }
        CExpression step = nullptr;
        if (!this->check(CTokenKind::RParen)) {
            step = this->parse_expression();
            if (!step) return nullptr;
        }
        if (!this->expect(CTokenKind::RParen,
                          CParserDiagnosticKind::ErrorExpectedToken)) {
            return nullptr;
        }
        ++m_loop_depth;
        auto body = this->parse_statement();
        --m_loop_depth;
        if (!body) return nullptr;
        auto range = span(start_tok->range, body->location());
        return make_node<ForStatement>(range, std::move(init),
                                       std::move(condition), std::move(step),
                                       std::move(body));
    }

    // return [ expr ] ;
    CStatement CParser::parse_return_statement() const {
        auto start_tok = this->expect(
            CTokenKind::Return, CParserDiagnosticKind::ErrorExpectedToken);

        CExpression expr = nullptr;
        if (!this->check(CTokenKind::Semicolon)) {
            expr = this->parse_expression();
            if (!expr) return nullptr;
        }
        auto semicolon = this->expect(
            CTokenKind::Semicolon, CParserDiagnosticKind::ErrorExpectedToken);
        if (!semicolon) return nullptr;
        auto range = span(start_tok->range, semicolon->range);
        return expr != nullptr
                   ? make_node<FlowStatement>(
                         range, Zaban::AST::Statements::FlowKind::Return,
                         std::move(expr))
                   : make_node<FlowStatement>(
                         range, Zaban::AST::Statements::FlowKind::Return);
    }

    // break ;
    CStatement CParser::parse_break_statement() const {
        // TODO: the loop depth check is related to semantic analysis and
        // could move to semantic layer later
        if (m_loop_depth == 0)
            this->report(CParserDiagnosticKind::ErrorBreakOutsideLoop);

        auto start_tok = this->expect(
            CTokenKind::Break, CParserDiagnosticKind::ErrorExpectedToken);

        auto semi = this->expect(CTokenKind::Semicolon,
                                 CParserDiagnosticKind::ErrorExpectedToken);
        if (!semi) return nullptr;
        auto range = span(start_tok->range, semi->range);

        return make_node<FlowStatement>(
            range, Zaban::AST::Statements::FlowKind::Break);

        // TODO: inside a switch, break is also valid. once switch exists,
        // track switch depth too (or a single "breakable" depth).
    }

    // continue ;
    CStatement CParser::parse_continue_statement() const {
        // TODO: the loop depth check is related to semantic analysis and
        // could move to semantic layer later
        if (m_loop_depth == 0)
            this->report(CParserDiagnosticKind::ErrorContinueOutsideLoop);

        auto start_tok = this->expect(
            CTokenKind::Continue, CParserDiagnosticKind::ErrorExpectedToken);

        auto semi = this->expect(CTokenKind::Semicolon,
                                 CParserDiagnosticKind::ErrorExpectedToken);
        if (!semi) return nullptr;
        auto range = span(start_tok->range, semi->range);

        return make_node<FlowStatement>(
            range, Zaban::AST::Statements::FlowKind::Continue);
    }

    CStatement CParser::parse_expression_statement() const {
        auto expr = this->parse_expression();
        if (!expr) return nullptr;

        auto semi = this->expect(CTokenKind::Semicolon,
                                 CParserDiagnosticKind::ErrorExpectedToken);
        if (!semi) return nullptr;

        const auto range = span(expr->location(), semi->range);
        return make_node<ExpressionStatement>(range, std::move(expr));
    }
}  // namespace Z::Zaban::Langs::CLang
