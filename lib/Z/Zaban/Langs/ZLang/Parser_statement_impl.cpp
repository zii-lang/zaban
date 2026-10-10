#include <Z/Zaban/Langs/ZLang/Parser.hpp>

namespace Z::Zaban::Langs::ZLang {
    AST::Statement<ZOffsetType> ZParser::parse_statement() const {
        auto token = this->m_stream.peek();
        switch (token->kind) {
            case ZTokenKind::Type:
            case ZTokenKind::Let: {
                auto decl = parse_declaration();
                //
            } break;
            default:
                break;
        }
        return nullptr;
    }
}  // namespace Z::Zaban::Langs::ZLang
