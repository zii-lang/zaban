#include <Z/Zaban/AST/Declaration.hpp>
#include <Z/Zaban/AST/Declarations/LetDeclaration.hpp>
#include <Z/Zaban/AST/Declarations/TypeDeclaration.hpp>
#include <Z/Zaban/Langs/ZLang/Parser.hpp>

#include "Z/Zaban/Config.hpp"

namespace Z::Zaban::Langs::ZLang {
    AST::Declaration<ZOffsetType> ZParser::parse_let_declaration() const {
        auto token = this->m_stream.peek();
        if (!this->m_stream.match(ZTokenKind::Let)) ZABAN_UNLIKELY {
                return nullptr;
            }
        auto start_position    = token->range.begin;
        auto identifier_atomic = this->parse_identifier_atomic();
        if (!identifier_atomic) {
            // TODO: report error expecting identifier after 'let'.
            return nullptr;
        }
        auto end_position = identifier_atomic->location().end;

        token = this->m_stream.peek();
        if (!token) {
            // TODO: report error unexpected eof.
            return nullptr;
        }

        AST::Annotation<ZOffsetType> annotation = nullptr;
        if (token->kind == ZTokenKind::Colon) {
            this->m_stream.advance();  // consume ':'
            annotation   = this->parse_annotation();
            end_position = annotation->location().end;
        }

        token = this->m_stream.peek();
        if (!token) {
            // TODO: report error unexpected eof.
            return nullptr;
        }

        AST::Expression<ZOffsetType> initializer = nullptr;
        if (token->kind == ZTokenKind::Equal) {
            this->m_stream.advance();  // consume '='
            initializer  = this->parse_expression();
            end_position = initializer->location().end;
        }

        AST::Declarations::LetDeclaration<ZOffsetType> let_declaration(
            identifier_atomic, annotation, initializer);
        let_declaration.set_location(
            OffsetRange<ZOffsetType>(start_position, end_position));
        return let_declaration.as_ptr();
    }

    AST::Declaration<ZOffsetType> ZParser::parse_type_declaration() const {
        auto token = this->m_stream.peek();
        if (!token || token->kind != ZTokenKind::Type) ZABAN_UNLIKELY {
                // TODO: report error.
                return nullptr;
            }
        auto start_position = token->range.begin;
        this->m_stream.advance();  // consume 'type'

        auto identifier_atomic = this->parse_identifier_atomic();
        if (!identifier_atomic) {
            // TODO: report type error required name for type declaration.
            return nullptr;
        }
        auto end_position = identifier_atomic->location().end;

        token = this->m_stream.peek();
        if (!token) {
            // TODO: report error unexpected eof.
            return nullptr;
        }

        AST::Annotation<ZOffsetType> annotation = nullptr;
        if (token->kind == ZTokenKind::Equal) {
            this->m_stream.advance();  // consume '='
            annotation   = this->parse_annotation();
            end_position = annotation->location().end;
        }

        AST::Declarations::TypeDeclaration<ZOffsetType> type_declaration(
            identifier_atomic, annotation);
        type_declaration.set_location(
            OffsetRange<ZOffsetType>(start_position, end_position));

        return type_declaration.as_ptr();
    }

    AST::Declaration<ZOffsetType> ZParser::parse_declaration() const {
        auto token = this->m_stream.peek();
        switch (token->kind) {
        ZABAN_LIKELY case ZTokenKind::Let:
            return parse_let_declaration();
        ZABAN_LIKELY case ZTokenKind::Type:
            return parse_type_declaration();
            default : return nullptr;
        }
        return nullptr;
    }
}  // namespace Z::Zaban::Langs::ZLang
