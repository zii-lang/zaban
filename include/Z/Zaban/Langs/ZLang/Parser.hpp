#pragma once

#include <Z/Zaban/AST/Declaration.hpp>
#include <Z/Zaban/AST/Declarations/LetDeclaration.hpp>
#include <Z/Zaban/AST/Declarations/TypeDeclaration.hpp>
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

        AST::Atomics::Identifier<ZOffsetType> parse_identifier_atomic();

        AST::Declaration<ZOffsetType> parse_declaration();
        std::shared_ptr<AST::Declarations::LetDeclaration<ZOffsetType>>
        parse_let_declaration();
        std::shared_ptr<AST::Declarations::TypeDeclaration<ZOffsetType>>
        parse_type_declaration();

        AST::Statement<ZOffsetType> parse_statement();

       public:
        ZParser(ZTokenStream);
        ZParser(ZTokenStream&&);
        AST::Module<ZOffsetType> parse();
    };
}  // namespace Z::Zaban::Langs::ZLang
