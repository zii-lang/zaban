#pragma once

#include <Z/Zaban/AST/Node.hpp>

namespace Z::Zaban::Langs::ZLang::AST {
	enum class ZNodeKind {
		ConditionCombinrator,
		WhenClause,
	};

	template<typename OffsetType = std::size_t>
	class ZNode : public Zaban::AST::Node<OffsetType> {
		public:
			virtual ZNodeKind z_node_kind() = 0;
	};
}
