#pragma once

#include <Z/Zaban/AST/Statement.hpp>
#include <Z/Zaban/Langs/ZLang/AST/Node.hpp>

namespace Z::Zaban::AST::ZLang {
    template<typename OffsetType = std::size_t>
    class ElseNode : public ZLang::Node<OffsetType> {
        using StatementType = Statement<OffsetType>;

       private:
        StatementType m_result;

       public:
        ElseNode(StatementType result) : m_result(result) {};
    };

    template<typename OffsetType = std::size_t>
    using ElseClause = std::shared_ptr<ElseNode<OffsetType>>;
}  // namespace Z::Zaban::AST::ZLang
