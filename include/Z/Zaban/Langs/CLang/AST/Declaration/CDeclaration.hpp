#pragma once

#include <Z/Zaban/AST/Annotation.hpp>
#include <Z/Zaban/AST/Atomic.hpp>
#include <Z/Zaban/AST/Declaration.hpp>
#include <Z/Zaban/AST/Declarations/LetDeclaration.hpp>
#include <Z/Zaban/AST/Expression.hpp>
#include <Z/Zaban/Langs/CLang/TokenKind.hpp>
#include <Z/Zaban/SourcePosition.hpp>
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <memory>
#include <utility>
#include <vector>

// TODO: having this inherit from let declaration is something we need to fix
// makes no sense to make every c decl a letdeclaration!
// must add an enum member to
// 'enum class DeclarationKind'
// like 'Extension' and have every extension use that instead + a kind
// of their own!
namespace Z::Zaban::Langs::CLang::AST::Declaration {
    template<typename OffsetType = std::size_t>
    struct StorageClass {
        CLang::TokenKind        kind;
        OffsetRange<OffsetType> range;
    };
    /** @brief A C declaration: a LetDeclaration plus its storage classes.
     *
     * Storage classes apply to the whole declaration, so every declarator in
     * `static int a, b;` gets its own copy. The parser only collects them in
     * source order; which combinations are valid is checked in sema.
     */
    template<typename OffsetType = std::size_t>
    class CLetDeclarationNode
        : public Zaban::AST::Declarations::LetDeclaration<OffsetType> {
        using Base = Zaban::AST::Declarations::LetDeclaration<OffsetType>;
        std::vector<StorageClass<OffsetType>> _storage;

       public:
        CLetDeclarationNode(Zaban::AST::Atomic<OffsetType>        name,
                            Zaban::AST::Annotation<OffsetType>    type,
                            Zaban::AST::Expression<OffsetType>    initializer,
                            std::vector<StorageClass<OffsetType>> storage) :
            Base(std::move(name), std::move(type), std::move(initializer)),
            _storage(std::move(storage)) {
        }

        const std::vector<StorageClass<OffsetType>>& get_storage_classes()
            const {
            return _storage;
        }

        bool has_storage_class(CLang::TokenKind kind) const {
            return std::ranges::any_of(
                _storage, [&](const auto& s) { return s.kind == kind; });
        }

        Zaban::AST::Declaration<OffsetType> as_ptr() const {
            return std::make_shared<CLetDeclarationNode<OffsetType>>(*this);
        }
    };
}  // namespace Z::Zaban::Langs::CLang::AST::Declaration
