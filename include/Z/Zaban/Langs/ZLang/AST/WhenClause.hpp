#pragma once

#include <Z/Zaban/AST/Statement.hpp>
#include <Z/Zaban/Langs/ZLang/AST/Condition.hpp>
#include <Z/Zaban/Langs/ZLang/AST/Node.hpp>

namespace Z::Zaban::AST::ZLang {
    template<typename OffsetType>
    class WhenNode;

    template<typename OffsetType = std::size_t>
    using WhenClause = std::shared_ptr<WhenNode<OffsetType>>;

    template<typename OffsetType = std::size_t>
    class WhenNode : public ZLang::Node<OffsetType> {
        using ConditionType = Condition<OffsetType>;
        using StatementType = Statement<OffsetType>;

       private:
        bool          m_is_idependent = false;
        ConditionType m_condition;
        StatementType m_result;

       public:
        WhenNode(bool is_independent, ConditionType condition,
                 StatementType result) :
            m_is_idependent(is_independent), m_condition(condition),
            m_result(result) {};

        NodeKind z_node_kind() const override {
            return NodeKind::ElseClause;
        }

        WhenClause<OffsetType> as_ptr() const {
            return std::make_shared<WhenNode<OffsetType>>(*this);
        }
    };
}  // namespace Z::Zaban::AST::ZLang
