#pragma once

#include <Z/Zaban/AST/Annotation.hpp>
#include <Z/Zaban/AST/Atomic.hpp>
#include <Z/Zaban/AST/Declaration.hpp>
#include <Z/Zaban/AST/Expression.hpp>
#include <cstdlib>
#include <memory>

namespace Z::Zaban::AST::Declarations {
    template<typename OffsetType = std::size_t>
    class LetDeclaration : public DeclarationNode<OffsetType> {
       private:
        const Atomic<OffsetType> m_identifier;

        // Optional type annotation.
        const Annotation<OffsetType> m_annotation = nullptr;

        // Optional initializer expression.
        const Expression<OffsetType> m_initializer = nullptr;

       public:
        /** @brief Creates a value declaration with only a name. */
        LetDeclaration(Atomic<OffsetType> identifier) :
            m_identifier(identifier) {};

        /** @brief Creates a value declaration with an explicit type. */
        LetDeclaration(Atomic<OffsetType>     identifier,
                       Annotation<OffsetType> annotation) :
            m_identifier(identifier), m_annotation(annotation) {};

        /** @brief Creates a value declaration with an initializer expression.
         */
        LetDeclaration(Atomic<OffsetType>     identifier,
                       Expression<OffsetType> initializer) :
            m_identifier(identifier), m_initializer(initializer) {};

        /** @brief Creates a value declaration with a type and initializer. */
        LetDeclaration(Atomic<OffsetType>     identifier,
                       Annotation<OffsetType> annotation,
                       Expression<OffsetType> initializer) :
            m_identifier(identifier), m_annotation(annotation),
            m_initializer(initializer) {};

        /** @brief Returns the declaration kind. */
        const DeclarationKind kind() const override {
            return DeclarationKind::LetDecl;
        }

        /** @brief Returns the declared value name. */
        const Atomic<OffsetType> get_identifier() const {
            return m_identifier;
        }

        /** @brief Returns the declared value type annotation, if available. */
        const Annotation<OffsetType> get_annotation() const {
            return m_annotation;
        }

        /** @brief Returns the initializer expression, if available. */
        const Expression<OffsetType> get_initializer() const {
            return m_initializer;
        }

        Declaration<OffsetType> as_ptr() {
            return std::make_shared<LetDeclaration<OffsetType>>(*this);
        }
    };
}  // namespace Z::Zaban::AST::Declarations
