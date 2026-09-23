#pragma once

#include <Z/Zaban/AST/Declaration.hpp>
#include <Z/Zaban/AST/Statement.hpp>

namespace Z::Zaban::AST::Statements {
    /** @brief Represents a declaration statement.
     *
     * DeclarationStatement wraps a declaration node so that declarations can
     * appear in statement sequences.
     */
    template<typename OffsetType = std::size_t>
    class DeclarationStatement : public StatementNode {
       private:
        const Declaration _declaration;

       public:
        /** @brief Creates a declaration statement. */
        DeclarationStatement(Declaration declaration) :
            _declaration(std::move(declaration)) {
        }

        StatementKind statement_kind() const override {
            return StatementKind::Declaration;
        }

        /** @brief Returns the wrapped declaration. */
        const Declaration get() const {
            return this->_declaration;
        }
    };
}  // namespace Z::Zaban::AST::Statements
