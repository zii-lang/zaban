#pragma once

#include <Z/Zaban/AST/Statement.hpp>

namespace Z::Zaban::Langs::CLang::AST::Statements {
    /** @brief Identifies the kind of C loop statement. */
    enum class LoopKind {
        /// A pre-condition loop (`while (cond) body`).
        While,
        /// A post-condition loop (`do body while (cond);`).
        DoWhile,
        /// A counted loop (`for (init; cond; step) body`).
        For,
    };

    /** @brief Base interface for C loop statements.
     *
     * LoopStatementNode groups the loop statements whose StatementKind is
     * Loop. Concrete classes identify their form through loop_kind().
     */
    template<typename OffsetType = std::size_t>
    class LoopStatementNode : public Zaban::AST::StatementNode<OffsetType> {
       public:
        /** @brief Virtual destructor for derived loop statement nodes. */
        virtual ~LoopStatementNode() = default;

        /** @brief Returns the loop kind. */
        virtual LoopKind loop_kind() const = 0;

        /** @brief Returns the statement kind. */
        Zaban::AST::StatementKind statement_kind() const override {
            return Zaban::AST::StatementKind::Loop;
        }
    };
}  // namespace Z::Zaban::Langs::CLang::AST::Statements
