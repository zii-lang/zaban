#pragma once

#include <Z/Zaban/AST/Statement.hpp>
#include <Z/Zaban/Langs/ZLang/AST/Extension/Condition.hpp>
#include <Z/Zaban/Langs/ZLang/AST/ZNode.hpp>

namespace Z::Zaban::Langs::ZLang::AST::Extension {
    template<typename OffsetType = std::size_t>
    class WhenNode : public Z::Zaban::Langs::ZLang::AST::ZNode<OffsetType> {
        using ConditionType =
            Langs::ZLang::AST::Extension::Condition<OffsetType>;
        using StatementType = Zaban::AST::Statement<OffsetType>;

       private:
        bool          m_is_idependent = false;
        ConditionType m_condition;
        StatementType m_result;

       public:
        WhenNode(bool is_independent, ConditionType condition,
                 StatementType result) :
            m_is_idependent(is_independent), m_condition(condition),
            m_result(result) {};
    };

    template<typename OffsetType = std::size_t>
    using WhenClause = std::shared_ptr<WhenNode<OffsetType>>;
}  // namespace Z::Zaban::Langs::ZLang::AST::Extension
