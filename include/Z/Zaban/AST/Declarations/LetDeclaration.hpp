#pragma once

#include <Z/Zaban/AST/Annotation.hpp>
#include <Z/Zaban/AST/Atomics/Identifier.hpp>
#include <Z/Zaban/AST/Declaration.hpp>
#include <Z/Zaban/AST/Expression.hpp>
#include <cstdlib>

namespace Z::Zaban::AST::Declarations {
    template<typename OffsetType = std::size_t>
    class LetDeclarationNode : public DeclarationNode<OffsetType> {
       private:
        const Atomics::Identifier<OffsetType> _name;

        // Optional type annotation.
        const Annotation<OffsetType> _type = nullptr;

        // Optional initializer expression.
        const Expression<OffsetType> _initializer = nullptr;

       public:
        /** @brief Creates a value declaration with only a name. */
        LetDeclarationNode(std::string name) : _name(name) {
        }

        /** @brief Creates a value declaration with an explicit type. */
        LetDeclarationNode(std::string name, Annotation<OffsetType> type) :
            _name(name), _type(std::move(type)), _initializer(nullptr) {
        }

        /** @brief Creates a value declaration with an initializer expression.
         */
        LetDeclarationNode(std::string name, Expression<OffsetType> init) :
            _name(name), _initializer(std::move(init)) {
        }

        /** @brief Creates a value declaration with a type and initializer. */
        LetDeclarationNode(std::string name, Annotation<OffsetType> type,
                           Expression<OffsetType> init) :
            _name(name), _type(std::move(type)), _initializer(std::move(init)) {
        }

        /** @brief Returns the declaration kind. */
        const DeclarationKind kind() const override {
            return DeclarationKind::LetDecl;
        }

        /** @brief Returns the declared value name. */
        const std::string get_name() const {
            return _name;
        }

        /** @brief Returns the declared value type annotation, if available. */
        const Annotation<OffsetType> get_annotation() const {
            return _type;
        }

        /** @brief Returns the initializer expression, if available. */
        const Expression<OffsetType> get_initializer() const {
            return _initializer;
        }
    };

    template<typename OffsetType = std::size_t>
    using LetDeclaration = std::shared_ptr<LetDeclarationNode<OffsetType>>;
}  // namespace Z::Zaban::AST::Declarations
