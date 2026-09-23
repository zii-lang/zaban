#pragma once

#include <Z/Zaban/AST/Statement.hpp>

namespace Z::Zaban::AST::Statements {
    /** @brief Represents a label statement.
     *
     * LabelStatement introduces a named control-flow target that can be
     * referenced by jump operations.
     */
    template<typename OffsetType = std::size_t>
    class LabelStatement : public StatementNode {
       private:
        const std::string _name;

       public:
        /** @brief Creates a label statement with the given name. */
        LabelStatement(std::string name) : _name(name) {
        }

        /** @brief Returns the statement kind. */
        StatementKind statement_kind() const override {
            return StatementKind::Label;
        }

        /** @brief Returns the label name. */
        std::string get_name() const {
            return this->_name;
        }
    };
}  // namespace Z::Zaban::AST::Statements
