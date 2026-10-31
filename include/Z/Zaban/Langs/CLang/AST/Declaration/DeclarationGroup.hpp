#pragma once

#include <Z/Zaban/AST/Declaration.hpp>
#include <utility>
#include <vector>

namespace Z::Zaban::Langs::CLang::AST::Declaration {
    template<typename OffsetType = std::size_t>
    class DeclarationGroupNode
        : public Zaban::AST::DeclarationNode<OffsetType> {
        std::vector<Zaban::AST::Declaration<OffsetType>> _declarations;

       public:
        explicit DeclarationGroupNode(
            std::vector<Zaban::AST::Declaration<OffsetType>> declarations) :
            _declarations(std::move(declarations)) {
        }

        const Zaban::AST::DeclarationKind kind() const override {
            return Zaban::AST::DeclarationKind::GroupDecl;
        }
        const std::vector<Zaban::AST::Declaration<OffsetType>>&
        get_declarations() const {
            return _declarations;
        }
    };
}  // namespace Z::Zaban::Langs::CLang::AST::Declaration
