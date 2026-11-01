#pragma once

#include <Z/Zaban/AST/Node.hpp>

namespace Z::Zaban::AST::ZLang {
    enum class NodeKind {
		Condition,
        HeaderConditions,
        ConditionCombinator,
        ElseClause,
        WhenClause,
    };

    template<typename OffsetType = std::size_t>
    class Node : public AST::Node<OffsetType> {
       public:
        virtual NodeKind z_node_kind() const = 0;
    };
}  // namespace Z::Zaban::AST::ZLang
