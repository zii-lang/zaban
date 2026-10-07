#include <Z/Zaban/AST/Annotation.hpp>
#include <Z/Zaban/AST/Annotations/ArrayAnnotation.hpp>
#include <Z/Zaban/AST/Annotations/ChainAnnotation.hpp>
#include <Z/Zaban/AST/Annotations/EnumAnnotation.hpp>
#include <Z/Zaban/AST/Annotations/PointerAnnotation.hpp>
#include <Z/Zaban/AST/Annotations/PrimitiveAnnotation.hpp>
#include <Z/Zaban/AST/Annotations/StructAnnotation.hpp>
#include <Z/Zaban/AST/Annotations/VarargAnnotation.hpp>
#include <Z/Zaban/AST/Expression.hpp>
#include <Z/Zaban/AST/Parameter.hpp>
#include <Z/Zaban/Config.hpp>
#include <Z/Zaban/Langs/ZLang/Parser.hpp>

namespace Z::Zaban::Langs::ZLang {
	AST::Annotation<ZOffsetType> ZParser::parse_variant_annotation() {
		return nullptr;
	}

    AST::Annotation<ZOffsetType> ZParser::parse_struct_annotation() {
        auto token = this->m_stream.peek();
        if (!token || token->kind != ZTokenKind::Struct) ZABAN_UNLIKELY {
                // TODO: this is an error and it's unlikely.
                return nullptr;
            }

        auto start_position = token->range.begin;  // struct begin.
        this->m_stream.advance();                  // consume "struct" token.

        if (!this->m_stream.match(ZTokenKind::RBrace)) {
            // TODO: this is an error, Required '{' after struct.
            return nullptr;
        }

        std::vector<AST::Parameter<ZOffsetType>> fields;

        while (true) {
            if (this->m_stream.match(ZTokenKind::RBrace)) {
                break;
            }

            AST::Parameter<ZOffsetType> field = this->parse_param();
            fields.emplace_back(field);

            if (this->m_stream.match(ZTokenKind::Comma)) {
                continue;
            }

            if (this->m_stream.match(ZTokenKind::RBrace)) {
                break;
            }

            // TODO: if we reached here. means enum field is incomplete.
            return nullptr;
        }
        auto end_position = this->m_stream.previous()->range.end;

        AST::Annotations::StructAnnotation<ZOffsetType> annotation(fields);
        annotation.set_location(
            OffsetRange<ZOffsetType>(start_position, end_position));

        return annotation.as_ptr();
    }

    AST::Annotation<ZOffsetType> ZParser::parse_enum_annotation() {
        auto token = this->m_stream.peek();
        if (!token || token->kind != ZTokenKind::Enum) ZABAN_UNLIKELY {
                // TODO: this is and error and unlikely.
                return nullptr;
            }
        auto start_position = token->range.begin;

        this->m_stream.advance();  // consume "enum" token.

        // match consumes "{" but throws error if not found or null.
        if (!this->m_stream.match(ZTokenKind::LBrace)) {
            // TODO: report error we need { after enum.
            return nullptr;
        }

        std::vector<AST::Parameter<ZOffsetType>> fields;

        while (true) {
            if (this->m_stream.match(ZTokenKind::RBrace)) {
                break;
            }

            AST::Parameter<ZOffsetType> field = this->parse_untyped_param();
            fields.emplace_back(field);

            if (this->m_stream.match(ZTokenKind::Comma)) {
                continue;
            }

            if (this->m_stream.match(ZTokenKind::RBrace)) {
                break;
            }

            // TODO: if we reached here. means enum field is incomplete.
            return nullptr;
        }
        auto end_position = this->m_stream.previous()->range.end;

        AST::Annotations::EnumAnnotation<ZOffsetType> annotation(fields);
        annotation.set_location(
            OffsetRange<ZOffsetType>(start_position, end_position));

        return annotation.as_ptr();
    }

    AST::Annotation<ZOffsetType> ZParser::parse_primary_annotation() {
        auto token       = this->m_stream.peek();
        auto start_token = token;

        if (!token) {
            // TODO: unexpected end of stream...
            return nullptr;
        }

        switch (token->kind) {
            case ZTokenKind::String: {
                AST::Atomic<ZOffsetType> prim_value =
                    this->parse_literal_atomic();
                AST::Annotations::PrimitiveAnnotation<ZOffsetType> annotation(
                    prim_value);
                return annotation.as_ptr();
            }
            case ZTokenKind::Identifier: {
                AST::Atomic<ZOffsetType> id_value =
                    this->parse_identifier_atomic();
                AST::Annotations::PrimitiveAnnotation<ZOffsetType> annotation(
                    id_value);
                return annotation.as_ptr();
            }
            case ZTokenKind::Enum: {
                return this->parse_enum_annotation();
            }
            case ZTokenKind::Struct: {
                return this->parse_struct_annotation();
            }
            case ZTokenKind::Vari: {
                return this->parse_variant_annotation();
            }
            case ZTokenKind::DDot: {
                this->m_stream.advance();
                AST::Annotations::VarargAnnotation<ZOffsetType> var_arg;
                var_arg.set_location(token->range);
                return var_arg.as_ptr();
            }
            default:
                // TODO: report error unexpected token for annotation.
                return nullptr;
        }
    }

    AST::Annotation<ZOffsetType> ZParser::parse_grouped_annotation() {
        auto token       = this->m_stream.peek();
        auto start_token = token;
        if (!token) {
            // TODO: unexpected end of line.
            return nullptr;
        }

        if (token->kind == ZTokenKind::LParen) {
            this->m_stream.advance();  // consume '('
            AST::Annotation<ZOffsetType> inner = this->parse_annotation();
            token                              = this->m_stream.peek();
            if (!token || token->kind != ZTokenKind::RParen) {
                // TODO: Error either eof or unclosed parentesis.
                return nullptr;
            }
            this->m_stream.advance();  // consume ')'
            // For groupped annotation we count open and close parantesis as
            // start and end location.
            inner->set_location(OffsetRange<ZOffsetType>(
                start_token->range.begin, token->range.end));
            return inner;
        }
        return this->parse_primary_annotation();
    }

    AST::Annotation<ZOffsetType> ZParser::parse_array_annotation() {
        auto underlaying_type = this->parse_grouped_annotation();
        if (!underlaying_type) {
            // TODO: probrably an error here cause we need to report this.
            return nullptr;
        }

        auto token = this->m_stream.peek();
        if (!token) {
            // might be error but don't report error.
            return underlaying_type;
        }

        if (token->kind == ZTokenKind::LBrak) {
            this->m_stream.advance();  // consume '['

            token = this->m_stream.peek();
            if (!token) {
                // ! TODO: Report Error.
                return nullptr;
            }

            if (token->kind == ZTokenKind::RBrak) {
                // TODO: we report error array type requires explicit size here
                // can be fixed during semantic analysis.
                return nullptr;
            }
            AST::Expression size_expr = this->parse_expression();

            token = this->m_stream.peek();
            if (!token || token->kind != ZTokenKind::RBrak) {
                // TODO: report we need to conusme ] at end of array annotation.
                return nullptr;
            }

            this->m_stream.advance();  // consume ]
            AST::Annotations::ArrayAnnotation<ZOffsetType> annotation(
                underlaying_type, size_expr);
            annotation.set_location(OffsetRange<ZOffsetType>(
                underlaying_type->location().begin, token->range.end));
            return annotation.as_ptr();
        }

        return underlaying_type;
    }

    AST::Annotation<ZOffsetType> ZParser::parse_pointer_annotation() {
        auto token = this->m_stream.peek();
        if (!token) {
            // TODO: report unexpected end of parse token.
            return nullptr;
        }
        if (token->kind == ZTokenKind::Asterisk) {
            this->m_stream.advance();
            AST::Annotation<ZOffsetType> pointee =
                this->parse_array_annotation();
            if (pointee == nullptr) {
                return nullptr;
            }

            AST::Annotations::PointerAnnotation<ZOffsetType> pointer(pointee);
            pointer.set_location(OffsetRange<ZOffsetType>(
                token->range.begin, pointee->location().end));
            return pointer.as_ptr();
        }

        return this->parse_array_annotation();
    }

    AST::Annotation<ZOffsetType> ZParser::parse_chain_annotation() {
        AST::Annotation<ZOffsetType> from = this->parse_pointer_annotation();
        if (from == nullptr) {
            return nullptr;
        }
        auto token = this->m_stream.peek();
        if (!token) {
            return from;
        }

        if (token->kind == ZTokenKind::Arrow) {
            this->m_stream.advance();  // consume '->'
            AST::Annotation<ZOffsetType> to = this->parse_chain_annotation();
            if (to == nullptr) {
                // TODO: report error we require some kind of annotation after
                // `->`.
                return nullptr;
            }
            AST::Annotations::ChainAnnotation<ZOffsetType> node(from, to);
            return node.as_ptr();
        }

        return from;
    }

    AST::Annotation<ZOffsetType> ZParser::parse_annotation() {
        return this->parse_chain_annotation();
    }
}  // namespace Z::Zaban::Langs::ZLang
