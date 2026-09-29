#pragma once

#include <Z/Zaban/AST/Atomic.hpp>
#include <Z/Zaban/AST/Declaration.hpp>
#include <Z/Zaban/AST/Module.hpp>
#include <Z/Zaban/AST/Statement.hpp>
#include <Z/Zaban/Langs/ZLang/Lexer.hpp>
#include <Z/Zaban/Langs/ZLang/TokenKind.hpp>
#include <Z/Zaban/Parse/Parser.hpp>
#include <Z/Zaban/Parse/TokenStream.hpp>

namespace Z::Zaban::Langs::ZLang {
    using ZOffsetType  = std::size_t;
    using ZTokenKind   = ZLang::TokenKind;
    using ZTokenType   = Lex::Token<ZTokenKind, ZOffsetType>;
    using ZTokenTypeP  = std::shared_ptr<ZTokenType>;
    using ZTokenStream = Parse::TokenStream<ZTokenKind, ZOffsetType>;

    class ZParser : public Parse::Parser<ZTokenKind, ZOffsetType> {
       private:
        ZTokenStream m_stream;

        AST::Atomic<ZOffsetType> parse_identifier_atomic();

        AST::Expression<ZOffsetType> parse_expression() const;
        AST::Expression<ZOffsetType> parse_assignment() const;
        AST::Expression<ZOffsetType> parse_logical_or() const;
        AST::Expression<ZOffsetType> parse_logical_and() const;
        AST::Expression<ZOffsetType> parse_bitwise_or() const;
        AST::Expression<ZOffsetType> parse_bitwise_xor() const;
        AST::Expression<ZOffsetType> parse_bitwise_and() const;
        AST::Expression<ZOffsetType> parse_equality() const;
        AST::Expression<ZOffsetType> parse_comparison() const;
        AST::Expression<ZOffsetType> parse_shifting() const;
        AST::Expression<ZOffsetType> parse_additive() const;
        AST::Expression<ZOffsetType> parse_multipicative() const;
        AST::Expression<ZOffsetType> parse_unary() const;
        AST::Expression<ZOffsetType> parse_suffix() const;

        AST::Declaration<ZOffsetType> parse_declaration();
        AST::Declaration<ZOffsetType> parse_let_declaration();
        AST::Declaration<ZOffsetType> parse_type_declaration();

        AST::Statement<ZOffsetType> parse_statement();

       public:
        ZParser(ZTokenStream);
        ZParser(ZTokenStream&&);
        AST::Module<ZOffsetType> parse();
    };
}  // namespace Z::Zaban::Langs::ZLang
