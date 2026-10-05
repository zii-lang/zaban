#pragma once

#include <Z/Zaban/AST/Node.hpp>
#include <memory>
#include <vector>

namespace Z::Zaban::AST {
    template<typename OffsetType = std::size_t>
    using Module = std::vector<std::shared_ptr<Node<OffsetType>>>;
}  // namespace Z::Zaban::AST
