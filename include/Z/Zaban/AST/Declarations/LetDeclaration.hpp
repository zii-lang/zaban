#pragma once

#include <Z/Zaban/AST/Annotation.hpp>
#include <Z/Zaban/AST/Declaration.hpp>
#include <Z/Zaban/AST/Expression.hpp>

namespace Z::Zaban::AST::Declarations {
    template<typename OffsetType = std::size_t>
    class TypeDeclaration : public DeclarationNode {
       private:
        const std::string _name;

        // Optional type annotation.
        const Annotation _type = nullptr;

        // Optional initializer expression.
        const Expression _initializer = nullptr;

       public:
        /** @brief Creates a value declaration with only a name. */
        LetDeclaration(std::string name) : _name(name) {
        }

        /** @brief Creates a value declaration with an explicit type. */
        LetDeclaration(std::string name, Annotation type) :
            _name(name), _type(std::move(type)), _initializer(nullptr) {
        }

        /** @brief Creates a value declaration with an initializer expression.
         */
        LetDeclaration(std::string name, Expr init) :
            _name(name), _initializer(std::move(init)) {
        }

        /** @brief Creates a value declaration with a type and initializer. */
        LetDeclaration(std::string name, Annotation type, Expr init) :
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
        const Annotation get_annotation() const {
            return _type;
        }

        /** @brief Returns the initializer expression, if available. */
        const Expression get_initializer() const {
            return _initializer;
        }
    };
}  // namespace Z::Zaban::AST::Declarations
