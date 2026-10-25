#pragma once

#include <Z/Zaban/AST/Expression.hpp>
#include <Z/Zaban/AST/Node.hpp>
#include <vector>

namespace Z::Zaban::Langs::ZLang::AST::Extension {
    template<typename OffsetType = std::size_t>
    class HeaderConditions : public Zaban::AST::Node<OffsetType> {
       private:
        std::vector<Zaban::AST::Expression<OffsetType>> headers{};

       public:
        OffsetRange<OffsetType> location() override {
            if (headers.empty()) {
                return OffsetRange<OffsetType>(0, 0);
            }

            return OffsetRange<OffsetType>(headers.front().location().begin,
                                           headers.back().location().end);
        }
    };
}  // namespace Z::Zaban::Langs::ZLang::AST::Abstract
