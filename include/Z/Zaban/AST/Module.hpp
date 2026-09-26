#pragma once

#include <Z/Zaban/AST/Node.hpp>
#include <vector>

namespace Z::Zaban::AST {
    template<typename OffsetType = std::size_t>
    using Module = std::vector<Node<OffsetType>>;
}  // namespace Z::Zaban::AST
