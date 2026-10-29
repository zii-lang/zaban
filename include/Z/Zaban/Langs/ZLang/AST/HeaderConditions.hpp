#pragma once

#include <Z/Zaban/AST/Expression.hpp>
#include <Z/Zaban/Langs/ZLang/AST/Node.hpp>
#include <vector>

namespace Z::Zaban::AST::ZLang {
    template<typename OffsetType = std::size_t>
    class HeaderConditions : public ZLang::Node<OffsetType> {
       private:
        std::vector<Expression<OffsetType>> headers{};

       public:
        OffsetRange<OffsetType> location() override {
            if (headers.empty()) {
                return OffsetRange<OffsetType>(0, 0);
            }

            return OffsetRange<OffsetType>(headers.front().location().begin,
                                           headers.back().location().end);
        }
    };
}  // namespace Z::Zaban::AST::ZLang
