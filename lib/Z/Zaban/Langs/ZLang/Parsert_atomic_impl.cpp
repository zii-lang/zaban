#include <Z/Zaban/AST/Atomics/Identifier.hpp>
#include <Z/Zaban/AST/Atomics/Literal.hpp>
#include <Z/Zaban/Langs/ZLang/Parser.hpp>

namespace Z::Zaban::Langs::ZLang {
    AST::Atomic<ZOffsetType> ZParser::parse_literal_atomic() const {
        auto token = this->m_stream.peek();

        if (token == nullptr) {
            // TODO: report unexpected eof.
            return nullptr;
        }

        AST::Atomics::Literal<ZOffsetType> literal;

        switch (token->kind) {
            case ZTokenKind::Null:
                literal = AST::Atomics::Literal<ZOffsetType>(
                    AST::Atomics::LiteralKind::Null, std::monostate{});
                literal.set_location(token->range);
                break;
            case ZTokenKind::True:
                literal = AST::Atomics::Literal<ZOffsetType>(
                    AST::Atomics::LiteralKind::Boolean, true);
                literal.set_location(token->range);
                break;
            case ZTokenKind::False:
                literal = AST::Atomics::Literal<ZOffsetType>(
                    AST::Atomics::LiteralKind::Boolean, false);
                literal.set_location(token->range);
                break;
            case ZTokenKind::Numeric:
                literal = AST::Atomics::Literal<ZOffsetType>(
                    AST::Atomics::LiteralKind::Numeric);
                literal.set_location(token->range);
                break;
            case ZTokenKind::String:
                literal = AST::Atomics::Literal<ZOffsetType>(
                    AST::Atomics::LiteralKind::String);
                literal.set_location(token->range);
                break;
            // TODO: left here...
            default:
                return nullptr;
        }

        return literal.get_ptr();
    }

    AST::Atomic<ZOffsetType> ZParser::parse_identifier_atomic() const {
        auto token = this->m_stream.peek();
        if (token == nullptr) {
            // TODO: report error.
            return nullptr;
        }

        if (token->kind == ZTokenKind::Identifier) {
            auto id_node = AST::Atomics::Identifier<ZOffsetType>(token->range);
            return id_node.get_ptr();
        }

        return nullptr;
    }
}  // namespace Z::Zaban::Langs::ZLang
