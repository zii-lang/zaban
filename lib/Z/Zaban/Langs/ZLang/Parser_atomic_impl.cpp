#include <Z/Zaban/AST/Atomics/Identifier.hpp>
#include <Z/Zaban/AST/Atomics/Literal.hpp>
#include <Z/Zaban/Langs/ZLang/Parser.hpp>

namespace Z::Zaban::Langs::ZLang {
    AST::Atomic<ZOffsetType> ZParser::parse_literal_atomic() const {
        auto token       = this->m_stream.peek();
        auto start_token = token;

        if (token == nullptr) {
            // TODO: report unexpected eof.
            return nullptr;
        }

        AST::Atomics::Literal<ZOffsetType> literal;

        switch (token->kind) {
            case ZTokenKind::Null:
                this->m_stream.advance();
                literal = AST::Atomics::Literal<ZOffsetType>(
                    AST::Atomics::LiteralKind::Null, std::monostate{});
                literal.set_location(token->range);
                break;
            case ZTokenKind::True:
                this->m_stream.advance();
                literal = AST::Atomics::Literal<ZOffsetType>(
                    AST::Atomics::LiteralKind::Boolean, true);
                literal.set_location(token->range);
                break;
            case ZTokenKind::False:
                this->m_stream.advance();
                literal = AST::Atomics::Literal<ZOffsetType>(
                    AST::Atomics::LiteralKind::Boolean, false);
                literal.set_location(token->range);
                break;
            case ZTokenKind::Numeric:
                this->m_stream.advance();
                literal = AST::Atomics::Literal<ZOffsetType>(
                    AST::Atomics::LiteralKind::Numeric);
                literal.set_location(token->range);
                break;
            case ZTokenKind::String:
                this->m_stream.advance();
                literal = AST::Atomics::Literal<ZOffsetType>(
                    AST::Atomics::LiteralKind::String);
                literal.set_location(token->range);
                break;
            case ZTokenKind::LBrak: {
                std::vector<AST::Expression<ZOffsetType>> elements{};
                this->m_stream.advance();
                token = this->m_stream.peek();

                while (!this->m_stream.end() &&
                       token->kind != ZTokenKind::RBrak) {
                    AST::Expression<ZOffsetType> element =
                        this->parse_expression();
                    if (element == nullptr) {
                        // TODO: Report Error, invalid syntax expecting
                        // expression in array.
                        break;
                    }
                    elements.emplace_back(element);

                    token = this->m_stream.peek();
                    if (token == nullptr) {
                        // TODO: report error.
                        return nullptr;
                    }

                    if (token->kind == ZTokenKind::Comma) {
                        this->m_stream.advance();
                        token = this->m_stream.peek();
                        continue;
                    } else if (token->kind != ZTokenKind::RBrak) {
                        // TODO: Report error.
                        break;
                    } else {
                        // this case can only be token->kind == RBrak so we
                        // consume it.
                        this->m_stream.advance();
                    }
                    literal = AST::Atomics::Literal<ZOffsetType>(
                        AST::Atomics::LiteralKind::Array, std::move(elements));
                    literal.set_location(OffsetRange<ZOffsetType>(
                        start_token->range.begin, token->range.end));
                }

                if (literal.get_kind() == AST::Atomics::LiteralKind::Null) {
                    // TODO: report error cause probably we got end-of-steam and
                    // didn't set literal.
                    return nullptr;
                }
            } break;
            case ZTokenKind::LBrace: {
                this->m_stream.advance();  // consume {

                while (true) {
                    token = this->m_stream.peek();
                    if (token == nullptr) {
                        // TODO: report error.
                        return nullptr;
                    }

                    if (token->kind == ZTokenKind::RBrace) {
                        break;
                    }
                }
                return nullptr;
            }
            default:
                return nullptr;
        }

        return literal.as_ptr();
    }

    AST::Atomic<ZOffsetType> ZParser::parse_identifier_atomic() const {
        auto token = this->m_stream.peek();
        if (token == nullptr) {
            // TODO: report error.
            return nullptr;
        }

        if (token->kind == ZTokenKind::Identifier) {
            this->m_stream.advance();
            auto id_node = AST::Atomics::Identifier<ZOffsetType>();
            id_node.set_location(token->range);
            return id_node.as_ptr();
        }

        return nullptr;
    }
}  // namespace Z::Zaban::Langs::ZLang
