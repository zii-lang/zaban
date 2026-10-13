#pragma once

#include <Z/Zaban/AST/Expression.hpp>
#include <vector>

namespace Z::Zaban::Langs::ZLang::AST {
    template<typename OffsetType = std::size_t>
    using HeaderCondition = std::vector<Z::Zaban::AST::Expression<OffsetType>>;
}
