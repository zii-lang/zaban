#pragma once

#include <Z/Zaban/AST/Statement.hpp>
#include <Z/Zaban/Langs/ZLang/AST/ZNode.hpp>

namespace Z::Zaban::Langs::ZLang::AST::Extension {
    template<typename OffsetType = std::size_t>
    class ElseNode : public Z::Zaban::Langs::ZLang::AST::ZNode<OffsetType> {
        using StatementType = Zaban::AST::Statement<OffsetType>;

       private:
        StatementType m_result;

       public:
        ElseNode(StatementType result) : m_result(result) {};
    };

    template<typename OffsetType = std::size_t>
	using ElseClause = std::shared_ptr<ElseNode<OffsetType>>;
}  // namespace Z::Zaban::Langs::ZLang::AST::Extension
