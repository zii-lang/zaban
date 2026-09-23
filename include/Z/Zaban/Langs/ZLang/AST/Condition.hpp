#pragma once

namespace Z::Zaban::Langs::ZLang {
    /** @brief Identifies how multiple conditions are combined within a
     * condition line.
     *
     * A ConditionCombinatorOperator defines the logical relationship between
     * individual condition expressions in a single conditional branch.
     *
     * Unlike normal boolean operators, these operators are part of Z's extended
     * conditional syntax and describe how multiple condition clauses are
     * evaluated.
     */
    enum class ConditionCombinatorOperator {
        /** @brief Logical AND condition composition (`?&`).
         *
         * All connected conditions must evaluate to true for the condition line
         * to match.
         *
         * Example:
         * @code
         * ?? == 20 ?& == 30
         * @endcode
         *
         * Equivalent to:
         * @code
         * A == 20 && B == 30
         * @endcode
         */
        And,
        /** @brief Logical OR condition composition (`?|`).
         *
         * At least one connected condition must evaluate to true for the
         * condition line to match.
         *
         * Example:
         * @code
         * ?? == 40 ?| == 20
         * @endcode
         *
         * Equivalent to:
         * @code
         * A == 40 || B == 20
         * @endcode
         */
        Or,
    };

    /** @brief Identifies the execution behavior of a conditional line.
     *
     * A ConditionLineOp controls how matching condition lines are related to
     * previous condition lines in the same conditional block.
     *
     * It extends the traditional if/else-if model by allowing independent
     * evaluation and parallel execution semantics.
     */
    enum class ConditionLineOperator {
        /** @brief Chain conditional chain (`??`).
         *
         * Represents a standard conditional branch. Only the first matching
         * condition in the chain is executed.
         *
         * Example:
         * @code
         * ?? == 20 ?& == 30 => { ... }
         * ?? == 40 ?| == 20 => { ... }
         * @endcode
         *
         * Equivalent to:
         * @code
         * if (A == 20 && B == 30) { ... }
         * else if (A == 40 || B == 20) { ... }
         * @endcode
         */
        Chain,
        /** @brief Serial independent conditional evaluation (`?!`).
         *
         * Each condition line is evaluated independently in sequence.
         * A successful condition does not prevent following conditions from
         * being evaluated.
         *
         * Example:
         * @code
         * ??  == 20 ?& == 30 => { ... }
         * ?!  == 40 ?| == 20 => { ... }
         * @endcode
         *
         * Equivalent to:
         * @code
         * if (A == 20 && B == 30) { ... }
         * if (A == 40 || B == 20) { ... }
         * @endcode
         */
        Serial,
        /** @brief Parallel conditional evaluation (`!!`).
         *
         * Each condition line is evaluated as an independent execution unit,
         * allowing matching branches to execute concurrently.
         *
         * Example:
         * @code
         * ?? == 20 ?& == 30 => { ... }
         * !! == 40 ?| == 20 => { ... }
         * @endcode
         *
         * Equivalent to:
         * @code
         * Thread(if (A == 20 && B == 30) { ... });
         * Thread(if (A == 40 || B == 20) { ... });
         * @endcode
         */
        Parallel,
    };

    /** @brief Defines how condition subject values are supplied to
     * half-condition expression.
     *
     * A ConditionArgMode controls the relationship between values declared in
     * the condition header and the arguments used by the expression evaluated
     * inside a condition line.
     *
     * Condition arguments allow conditions to invoke functions
     * using values captured by the surrounding condition statement.
     *
     * Example:
     * @code
     * if a, b
     *     ?? [] add == 30 => { ... }
     * endif;
     * @endcode
     *
     * The values `a` and `b` are passed according to the selected mode.
     */
    enum class ConditionArgMode {
        /** @brief No explicit argument passing mode.
         *
         * Represents a regular condition where the condition expression does
         * not receive values from the condition header.
         */
        None,
        /** @brief Pass all condition header values in declaration order (`[]`).
         *
         * All values from the condition header are forwarded to the expression
         * in their original order.
         *
         * Example:
         * @code
         * if a, b
         *     ?? [] add == 30 => { ... }
         * endif;
         * @endcode
         *
         * Equivalent to:
         * @code
         * add(a, b) == 30
         * @endcode
         */
        PassAll,
        /** @brief Pass selected condition header values by position
         * (`(index...)`).
         *
         * Explicit indexes determine which values from the condition header are
         * forwarded to the expression and in which order.
         *
         * Indexes refer to the position of values in the condition header.
         *
         * Example:
         * @code
         * if a, b
         *     ?? (1, 0) sub == 20 => { ... }
         * endif;
         * @endcode
         *
         * Equivalent to:
         * @code
         * sub(b, a) == 20
         * @endcode
         */
        Positional,
        /** @brief Pass explicitly specified expressions (`[...]`).
         *
         * Instead of forwarding values from the condition header, explicit
         * expressions are evaluated and passed as arguments.
         *
         * Example:
         * @code
         * if a, b
         *     ?? [a, b, 5] sum3 == 35 => { ... }
         * endif;
         * @endcode
         *
         * Equivalent to:
         * @code
         * sum3(a, b, 5) == 35
         * @endcode
         */
        Exact,
    };
}  // namespace Z::Zaban::Langs::ZLang
