#pragma once

#include <Z/Zaban/AST/Expression.hpp>
#include <Z/Zaban/AST/Node.hpp>
#include <memory>
#include <vector>

namespace Z::Zaban::Langs::ZLang::ExtAST {
    template<typename OffsetType = std::size_t>
    using HeaderConditions = std::vector<AST::Expression<OffsetType>>;

    template<typename OffsetType = std::size_t>
    class HeaderConditionNode;

    template<typename OffsetType = std::size_t>
    using HeaderCondition = std::shared_ptr<HeaderConditionNode<OffsetType>>;

    template<typename OffsetType>
    class HeaderConditionNode : public AST::Node<OffsetType> {
       private:
        HeaderConditions<OffsetType> m_conditions;

       public:
        HeaderConditionNode(HeaderConditions<OffsetType> conditions) :
            m_conditions(conditions) {};

        HeaderConditions<OffsetType> get_conditions() const {
            return this->m_conditions;
        }

        std::shared_ptr<HeaderConditionNode<OffsetType>> as_ptr() {
            return std::make_shared<HeaderConditionNode<OffsetType>>(*this);
        }
    };
}  // namespace Z::Zaban::Langs::ZLang::ExtAST
