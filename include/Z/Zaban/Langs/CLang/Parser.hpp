#pragma once

#include <Z/Zaban/AST/Expression.hpp>
#include <Z/Zaban/AST/Module.hpp>
#include <Z/Zaban/Langs/CLang/Lexer.hpp>
#include <Z/Zaban/Langs/CLang/TokenKind.hpp>
#include <Z/Zaban/Parse/Parser.hpp>
#include <Z/Zaban/Parse/TokenStream.hpp>
#include <cstddef>

#include "Z/Zaban/AST/Declaration.hpp"
#include "Z/Zaban/AST/Statement.hpp"

namespace Z::Zaban::Langs::CLang {
    using COffsetType   = std::size_t;
    using CTokenKind    = CLang::TokenKind;
    using CTokenType    = Lex::Token<CTokenKind, COffsetType>;
    using CTokenTypePtr = std::shared_ptr<CTokenType>;
    using CTokenStream  = Parse::TokenStream<CTokenKind, COffsetType>;

    class CParser : public Parse::Parser<CTokenKind, COffsetType> {
       private:
        CTokenStream m_stream;

        AST::Expression<COffsetType> parse_expression() const;
        AST::Expression<COffsetType> parse_assignment() const;
        AST::Expression<COffsetType> parse_logical_or() const;
        AST::Expression<COffsetType> parse_logical_and() const;
        AST::Expression<COffsetType> parse_bitwise_or() const;
        AST::Expression<COffsetType> parse_bitwise_xor() const;
        AST::Expression<COffsetType> parse_bitwise_and() const;
        AST::Expression<COffsetType> parse_equality() const;
        AST::Expression<COffsetType> parse_comparison() const;
        AST::Expression<COffsetType> parse_shifting() const;
        AST::Expression<COffsetType> parse_additive() const;
        AST::Expression<COffsetType> parse_multipicative() const;
        AST::Expression<COffsetType> parse_unary() const;
        AST::Expression<COffsetType> parse_suffix() const;
        AST::Expression<COffsetType> parse_primary() const;
        AST::Expression<COffsetType> parse_group() const;

        AST::Declaration<COffsetType> parse_declaration() const;
        AST::Declaration<COffsetType> parse_types() const;

        AST::Statement<COffsetType> parse_statement() const;

        AST::Module<COffsetType> parse(CTokenStream stream) override;
    };

}  // namespace Z::Zaban::Langs::CLang
