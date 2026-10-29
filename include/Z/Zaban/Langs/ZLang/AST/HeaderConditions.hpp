#pragma once

#include <Z/Zaban/AST/Expression.hpp>
#include <Z/Zaban/Langs/ZLang/AST/Node.hpp>
#include <vector>

namespace Z::Zaban::AST::ZLang {
    template<typename OffsetType>
    class HeaderConditionsNode;

    template<typename OffsetType = std::size_t>
    using HeaderConditions = std::shared_ptr<HeaderConditionsNode<OffsetType>>;

    template<typename OffsetType = std::size_t>
    class HeaderConditionsNode : public ZLang::Node<OffsetType> {
       private:
        std::vector<Expression<OffsetType>> m_headers;

       public:
        HeaderConditionsNode() : m_headers({}) {};
        HeaderConditionsNode(Expression<OffsetType> expr) : m_headers({}) {
            m_headers.emplace_back(expr);
        };
        HeaderConditionsNode(std::vector<Expression<OffsetType>> headers) :
            m_headers(headers) {};

        OffsetRange<OffsetType> location() const override {
            if (m_headers.empty()) {
                return OffsetRange<OffsetType>(0, 0);
            }

            return OffsetRange<OffsetType>(m_headers.front().location().begin,
                                           m_headers.back().location().end);
        }

        NodeKind z_node_kind() const override {
            return NodeKind::HeaderConditions;
        }

        HeaderConditions<OffsetType> as_ptr() const {
            return std::make_shared<HeaderConditionsNode<OffsetType>>(*this);
        }
    };
}  // namespace Z::Zaban::AST::ZLang
