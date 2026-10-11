#pragma once

#include <Z/Zaban/AST/Atomic.hpp>
#include <Z/Zaban/AST/Statement.hpp>

namespace Z::Zaban::AST::Statements {
    /** @brief Represents a label statement.
     *
     * LabelStatement introduces a named control-flow target that can be
     * referenced by jump operations.
     */
    template<typename OffsetType = std::size_t>
    class LabelStatement : public StatementNode<OffsetType> {
       private:
        const Atomic<OffsetType> m_id;

       public:
        /** @brief Creates a label statement with the given id. */
        LabelStatement(Atomic<OffsetType> id) : m_id(id) {
        }

        /** @brief Returns the statement kind. */
        StatementKind statement_kind() const override {
            return StatementKind::Label;
        }

        /** @brief Returns the label name. */
        Atomic<OffsetType> get_id() const {
            return this->m_id;
        }

        Statement<OffsetType> as_ptr() {
            return std::make_shared<LabelStatement<OffsetType>>(*this);
        }
    };
}  // namespace Z::Zaban::AST::Statements
