#pragma once

#include <Z/Zaban/Langs/ZLang/AST/ZNode.hpp>

namespace Z::Zaban::AST::Langs::ZLang::AST::Extension {
    template<typename OffsetType = std::size_t>
    class WhenClause : public Z::Zaban::Langs::ZLang::AST::ZNode<OffsetType> {};
}  // namespace Z::Zaban::AST::Langs::ZLang::AST::Extension
