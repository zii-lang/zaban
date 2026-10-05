#include <Z/Zaban/AST/Declaration.hpp>
#include <Z/Zaban/Langs/ZLang/Parser.hpp>

namespace Z::Zaban::Langs::ZLang {
    AST::Declaration<ZOffsetType> ZParser::parse_let_declaration() {
        auto token = this->m_stream.peek();
        if (token == nullptr) {
            // TODO: report unterminated let declaration.
            return nullptr;
        }
        if (token->kind != ZTokenKind::Identifier) {
            // TODO: report error expected identifier.
            return nullptr;
        }

        return nullptr;
    }

    AST::Declaration<ZOffsetType> ZParser::parse_type_declaration() {
        return nullptr;
    }
}  // namespace Z::Zaban::Langs::ZLang
