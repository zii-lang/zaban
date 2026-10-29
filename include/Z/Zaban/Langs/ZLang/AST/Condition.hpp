#pragma once

#include <Z/Zaban/AST/Atomic.hpp>
#include <Z/Zaban/AST/Expression.hpp>
#include <Z/Zaban/AST/Node.hpp>              // Node
#include <Z/Zaban/Langs/ZLang/AST/Node.hpp>  // ZLang::Node
#include <memory>
#include <optional>
#include <variant>
#include <vector>

namespace Z::Zaban::AST::ZLang {
    enum class ComparisionOperator {
        Equal,
        NEqual,
        Lesser,
        LesserEqual,
        Greater,
        GreaterEqual,
    };

    enum class FunctionConditionKind {
        PassAll,
        Exact,
        Indexed,
    };

    enum class ConditionKind {
        Half,
        Full,
        Binary,
    };

    template<typename OffsetType = std::size_t>
    class ConditionNode : public ZLang::Node<OffsetType> {
       public:
        virtual ConditionKind get_condition_kind() = 0;
    };

    template<typename OffsetType = std::size_t>
    class FunctionConditionNode : public ZLang::Node<OffsetType> {
       protected:
        AST::Atomic<OffsetType> m_id;

       public:
        virtual FunctionConditionKind get_function_condition_kind() = 0;
        AST::Atomic<OffsetType>       get_id() {
            return this->m_id;
        }
    };

    template<typename OffsetType = std::size_t>
    using Condition = std::shared_ptr<ConditionNode<OffsetType>>;

    template<typename OffsetType = std::size_t>
    using FunctionCondition =
        std::shared_ptr<FunctionConditionNode<OffsetType>>;

    template<typename OffsetType = std::size_t>
    class PassAllConditionNode : public FunctionConditionNode<OffsetType> {
       public:
        FunctionConditionKind get_function_condition_kind() override {
            return FunctionConditionKind::PassAll;
        }
    };

    template<typename OffsetType = std::size_t>
    class ExactConditionNode : public FunctionConditionNode<OffsetType> {
       private:
        std::vector<AST::Expression<OffsetType>> m_expressions;

       public:
        FunctionConditionKind get_function_condition_kind() override {
            return FunctionConditionKind::Exact;
        }
    };

    template<typename OffsetType = std::size_t>
    class IndexedConditionNode : public FunctionConditionNode<OffsetType> {
       private:
        std::vector<AST::Expression<OffsetType>> m_indices;

       public:
        FunctionConditionKind get_function_condition_kind() override {
            return FunctionConditionKind::Indexed;
        }
    };

    template<typename OffsetType = std::size_t>
    class HalfCondition : public ConditionNode<OffsetType> {
       private:
        ComparisionOperator         m_operator;
        AST::Expression<OffsetType> m_comparie;

       public:
        ConditionKind get_condition_kind() {
            return ConditionKind::Half;
        }
    };

    template<typename OffsetType = std::size_t>
    class FullCondition : public ConditionNode<OffsetType> {
       private:
        std::variant<AST::Expression<OffsetType>, FunctionCondition<OffsetType>>
                                           rhs;
        std::optional<ComparisionOperator> op;
        AST::Expression<OffsetType>        lhs;
    };

    enum class ConditionCombinatorOperator {
        And,
        Or,
    };

    template<typename OffsetType = std::size_t>
    class ConditionCombinatorNode : public ConditionNode<OffsetType> {
       private:
        Condition<OffsetType>       lhs;
        ConditionCombinatorOperator op;
        Condition<OffsetType>       rhs;

       public:
        ConditionKind get_condition_kind() override {
            return ConditionKind::Binary;
        }
    };
}  // namespace Z::Zaban::AST::ZLang
