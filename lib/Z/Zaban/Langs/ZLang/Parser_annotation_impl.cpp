#include <Z/Zaban/AST/Annotation.hpp>
#include <Z/Zaban/AST/Annotations/ChainAnnotation.hpp>
#include <Z/Zaban/Langs/ZLang/Parser.hpp>

namespace Z::Zaban::Langs::ZLang {

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
            return node.get_ptr();
        }

        return from;
    }

    AST::Annotation<ZOffsetType> ZParser::parse_annotation() {
        return this->parse_chain_annotation();
    }
}  // namespace Z::Zaban::Langs::ZLang
