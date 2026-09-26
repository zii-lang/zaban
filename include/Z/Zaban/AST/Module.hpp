#pragma once

#include <Z/Zaban/AST/Node.hpp>
#include <vector>

namespace Z::Zaban::AST {
    template<typename OffsetType = std::size_t>
    using Module = std::vector<Node>;
}  // namespace Z::Zaban::AST
