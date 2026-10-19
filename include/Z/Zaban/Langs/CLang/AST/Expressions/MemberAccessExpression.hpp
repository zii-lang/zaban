#pragma once

#include <Z/Zaban/AST/Atomic.hpp>
#include <Z/Zaban/AST/Expression.hpp>
#include <Z/Zaban/AST/Expressions/MemberAccessExpression.hpp>
#include <memory>

namespace Z::Zaban::Langs::CLang::AST::Expressions {
    /** @brief Represents a C member access expression.
     *
     * Extends the shared member access node with which operator was used, so
     * code that only cares about member access can treat it as the base node
     * while C-aware code can tell `.` from `->`.
     *
     * Example:
     * @code
     * point.x
     * node->next
     * @endcode
     */
    template<typename OffsetType = std::size_t>
    class MemberAccessExpressionNode :
        public Zaban::AST::Expressions::MemberAccessExpression<OffsetType> {
        const bool _is_arrow;

       public:
        /** @brief Creates a member access expression. */
        MemberAccessExpressionNode(Zaban::AST::Expression<OffsetType> base,
                                   Zaban::AST::Atomic<OffsetType>     member,
                                   bool                               is_arrow) :
            Zaban::AST::Expressions::MemberAccessExpression<OffsetType>(
                std::move(base), std::move(member)),
            _is_arrow(is_arrow) {
        }

        /** @brief Returns whether the access used `->` rather than `.`. */
        bool is_arrow() const {
            return this->_is_arrow;
        }
    };

    template<typename OffsetType = std::size_t>
    using MemberAccessExpression =
        std::shared_ptr<MemberAccessExpressionNode<OffsetType>>;
}  // namespace Z::Zaban::Langs::CLang::AST::Expressions
