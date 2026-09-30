#include <Z/Zaban/AST/Atomics/Identifier.hpp>
#include <Z/Zaban/Langs/ZLang/Parser.hpp>

namespace Z::Zaban::Langs::ZLang {
    AST::Atomic<ZOffsetType> ZParser::parse_literal_atomic() const {
        auto token = this->m_stream.peek();

        if (token == nullptr) {
            // TODO: report unexpected eof.
            return nullptr;
        }

        return nullptr;
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
