#pragma once

namespace Z::Zaban::AST {
    /** @brief Identifies a boolean comparison operation.
     *
     * Boolean operators represent relational operations that compare two values
     * and produce a boolean result.
     *
     * Unlike BinraryOperator, BooleanOperator contains only operations whose
     * result is a logical truth value.
     */
    enum class BooleanOperator {
        /// Equality comparison operator (`==`).
        Eq,
        /// Inequality comparison operator (`!=`).
        Neq,
        /// Less-than comparison operator (`<`).
        Lt,
        /// Greater-than comparison operator (`>`).
        Gt,
        /// Less-than-or-equal comparison operator (`<=`).
        Lte,
        /// Greater-than-or-equal comparison operator (`>=`).
        Gte,
    };
}  // namespace Z::Zaban::AST
