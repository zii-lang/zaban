#pragma once

#include <Z/Zaban/AST/Expression.hpp>
#include <memory>
#include <vector>

namespace Z::Zaban::Langs::CLang::AST::Expressions {
    /** @brief Represents a brace-enclosed C initializer list.
     *
     * Whether the list initializes an array or a structure is not known while
     * parsing, so elements are stored as plain expressions and matched against
     * the declared type during semantic analysis. Nested lists are stored as
     * nested InitializerListExpressionNode elements.
     *
     * Example:
     * @code
     * int a[2][2] = {{1, 2}, {3, 4}};
     * @endcode
     */
    template<typename OffsetType = std::size_t>
    class InitializerListExpressionNode
        : public Zaban::AST::ExpressionNode<OffsetType> {
        std::vector<Zaban::AST::Expression<OffsetType>> _elements;

       public:
        /** @brief Creates an initializer list from its elements. */
        InitializerListExpressionNode(
            std::vector<Zaban::AST::Expression<OffsetType>>&& elements) :
            _elements(std::move(elements)) {
        }

        /** @brief Returns the expression kind. */
        Zaban::AST::ExpressionKind expr_kind() const override {
            return Zaban::AST::ExpressionKind::InitializerList;
        }

        /** @brief Returns the number of elements. */
        std::size_t get_elementc() const {
            return this->_elements.size();
        }

        /** @brief Returns the element at the specified position. */
        Zaban::AST::Expression<OffsetType> get_element_at(
            std::size_t pos) const {
            return this->_elements[pos];
        }

        /** @brief Returns an iterator to the first element. */
        typename std::vector<Zaban::AST::Expression<OffsetType>>::const_iterator
        element_begin() const {
            return this->_elements.begin();
        }

        /** @brief Returns an iterator past the last element. */
        typename std::vector<Zaban::AST::Expression<OffsetType>>::const_iterator
        element_end() const {
            return this->_elements.end();
        }
    };

    template<typename OffsetType = std::size_t>
    using InitializerListExpression =
        std::shared_ptr<InitializerListExpressionNode<OffsetType>>;
}  // namespace Z::Zaban::Langs::CLang::AST::Expressions
