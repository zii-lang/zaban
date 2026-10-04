#pragma once

#include <Z/Zaban/AST/Statement.hpp>
#include <vector>

namespace Z::Zaban::AST::Statements {
    /** @brief Represents a scoped sequence of statements.
     *
     * BlockStatement contains an ordered collection of statements that are
     * evaluated sequentially within a lexical scope.
     *
     * Blocks are used to group statements together and define scope boundaries
     * for declarations and name resolution.
     */
    template<typename OffsetType = std::size_t>
    class BlockStatement : public StatementNode<OffsetType> {
       private:
        std::vector<Statement<OffsetType>> _statements;

       public:
        /** @brief Creates a block statement from a list of statements. */
        BlockStatement(std::vector<Statement<OffsetType>> statements) :
            _statements(std::move(statements)) {
        }

        /** @brief Returns the number of statements contained in this block. */
        std::size_t statementc() const {
            return this->_statements.size();
        }

        /** @brief Returns an iterator to the first statement in the block. */
        typename std::vector<Statement<OffsetType>>::iterator stmt_begin() {
            return this->_statements.begin();
        }

        /** @brief Returns an iterator past the last statement in the block. */
        typename std::vector<Statement<OffsetType>>::iterator stmt_end() {
            return this->_statements.end();
        }

        /** @brief Returns the statement at the specified index. */
        Statement<OffsetType> get_statement(std::size_t pos) {
            return this->_statements[pos];
        }

        /** @brief Returns the statement kind. */
        StatementKind statement_kind() const override {
            return StatementKind::Block;
        }
    };
}  // namespace Z::Zaban::AST::Statements
