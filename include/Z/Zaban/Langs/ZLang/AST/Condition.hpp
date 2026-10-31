#pragma once

#include <Z/Zaban/AST/Atomic.hpp>
#include <Z/Zaban/AST/Condition.hpp>
#include <Z/Zaban/AST/Expression.hpp>
#include <Z/Zaban/AST/Node.hpp>              // Node
#include <Z/Zaban/Langs/ZLang/AST/Node.hpp>  // ZLang::Node
#include <memory>
#include <optional>
#include <variant>
#include <vector>

namespace Z::Zaban::AST::ZLang {
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
        NodeKind z_node_kind() const override {
            return NodeKind::Condition;
        }
        virtual ConditionKind get_condition_kind() const = 0;
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
        BooleanOperator             m_operator;
        AST::Expression<OffsetType> m_comparie;

       public:
        explicit HalfCondition(BooleanOperator             op,
                               AST::Expression<OffsetType> expr) :
            m_operator(op), m_comparie(expr) {};

        ConditionKind get_condition_kind() const override {
            return ConditionKind::Half;
        }

        Condition<OffsetType> as_ptr() const {
            return std::make_shared<HalfCondition<OffsetType>>(*this);
        }
    };

    template<typename OffsetType = std::size_t>
    class FullCondition : public ConditionNode<OffsetType> {
       private:
        std::variant<AST::Expression<OffsetType>, FunctionCondition<OffsetType>>
                                       lhs;
        std::optional<BooleanOperator> op;
        AST::Expression<OffsetType>    rhs;
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
