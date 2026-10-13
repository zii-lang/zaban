#include <Z/Zaban/AST/Parameter.hpp>
#include <Z/Zaban/Langs/ZLang/Parser.hpp>

namespace Z::Zaban::Langs::ZLang {
    AST::Parameter<ZOffsetType> ZParser::parse_untyped_param() const {
        auto token       = this->m_stream.peek();
        auto start_token = token;

        if (token == nullptr) {
            // TODO: we got end of token stream.
            return nullptr;
        }

        auto identifier = this->parse_identifier_atomic();
        if (identifier == nullptr) {
            // TODO: we need identifier and could not do it.
            return nullptr;
        }

        token = this->m_stream.peek();
        if (token == nullptr) {
            // TODO: we got end of token.
            return nullptr;
        }

        if (token->kind == ZTokenKind::Equal) {
            this->m_stream.advance();  // consume =
            auto expression = this->parse_expression();
            if (expression == nullptr) {
                // we probably got an error before while parsing expression so
                // don't report again.
                return nullptr;
            }

            AST::ParameterNode<ZOffsetType> param(identifier, expression);
            param.set_location(OffsetRange<ZOffsetType>(
                start_token->range.begin, expression->location().end));
            return std::make_shared<AST::ParameterNode<ZOffsetType>>(param);
        }

        AST::ParameterNode<ZOffsetType> param(identifier, false);
        param.set_location(identifier->location());
        return std::make_shared<AST::ParameterNode<ZOffsetType>>(param);
    }

    AST::Parameter<ZOffsetType> ZParser::parse_param() const {
        auto token       = this->m_stream.peek();
        auto start_token = token;

        if (token == nullptr) {
            // TODO: unexpected end-of-file.
            return nullptr;
        }

        if (token->kind == ZTokenKind::Identifier) {
            AST::Atomic identifier = this->parse_identifier_atomic();
            if (identifier == nullptr) ZABAN_UNLIKELY {
                    // TODO: we already have identifier, but didn't parse atomic
                    // for it.
                    return nullptr;
                }
            AST::Annotation annotation  = nullptr;
            AST::Expression initializer = nullptr;

            token = this->m_stream.peek();
            if (!token) {
                // TODO: report fail, null token. eof.
                return nullptr;
            }

            if (token->kind == ZTokenKind::Colon) {
                this->m_stream.advance();  // consume :
                // annotation = this->parse_annotation();
            }
            token = this->m_stream.peek();
            if (!token) {
                // TODO: report fail, null token. eof.
                return nullptr;
            }
            if (token->kind == ZTokenKind::Equal) {
                this->m_stream.advance();  // consume =
                initializer = this->parse_expression();
                if (!initializer) {
                    // TODO: report error we need initializer after '=' in
                    // parameter.
                    return nullptr;
                }
            }
            ZOffsetType start_range = identifier->location().begin;
            ZOffsetType end_range   = 0;
            if (!initializer && !annotation) {
                end_range = identifier->location().end;
            } else if (!initializer) {
                end_range = annotation->location().end;
            } else {
                end_range = initializer->location().end;
            }

            AST::ParameterNode<ZOffsetType> param(identifier, annotation,
                                                  initializer);
            param.set_location(
                OffsetRange<ZOffsetType>(start_range, end_range));
            return std::make_shared<AST::ParameterNode<ZOffsetType>>(param);
        } else if (token->kind == ZTokenKind::DDot) {
            this->m_stream.advance();  // consume `..`

            auto identifier = this->parse_identifier_atomic();
            if (identifier == nullptr) {
                // TODO: Error missing token expecting identifier for variable
                // arg argument.
                return nullptr;
            }

            AST::ParameterNode<ZOffsetType> param(identifier, true);
            param.set_location(OffsetRange<ZOffsetType>(
                token->range.begin, identifier->location().end));
            return std::make_shared<AST::ParameterNode<ZOffsetType>>(param);
        } else {
            // TODO: Report error Expected 'identifier' or variable argument for
            // parameter.
            return nullptr;
        }
    }

}  // namespace Z::Zaban::Langs::ZLang
