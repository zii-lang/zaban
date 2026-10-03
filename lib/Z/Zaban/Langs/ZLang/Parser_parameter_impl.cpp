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

        auto identifier_atomic = this->parse_identifier_atomic();
        if (identifier_atomic == nullptr) {
            // TODO: we need identifier and could not do it.
            return nullptr;
        }

        auto identifier =
            *identifier_atomic
                 ->dyn_cast<AST::Atomics::Identifier<ZOffsetType>>()
                 .get();

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
            return param.get_ptr();
        }

        AST::ParameterNode<ZOffsetType> param(identifier, false);
        param.set_location(identifier_atomic->location());
        return param.get_ptr();
    }

    AST::Parameter<ZOffsetType> ZParser::parse_param() const {
        return nullptr;
    }

}  // namespace Z::Zaban::Langs::ZLang
