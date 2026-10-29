#pragma once

#include <Z/Zaban/AST/Node.hpp>

namespace Z::Zaban::AST::ZLang {
    enum class ZNodeKind {
        HeaderConditions,
        ConditionCombinator,
        ElseClause,
        WhenClause,
    };

    template<typename OffsetType = std::size_t>
    class Node : public AST::Node<OffsetType> {
       public:
        virtual ZNodeKind z_node_kind() = 0;
    };
}  // namespace Z::Zaban::AST::ZLang
