#pragma once

// system includes
#include <cstddef>
// zaban includes
#include <Z/Zaban/Langs/ZLang/TokenKind.hpp>

namespace Z::Zaban::Langs::ZLang {

    struct TokenPair {
        ZLang::TokenKind lhs;
        ZLang::TokenKind rhs;

        bool operator==(const TokenPair&) const = default;
    };

    struct TokenPairHash {
        std::size_t operator()(const TokenPair& pair) const noexcept {
            const auto lhs = static_cast<std::size_t>(pair.lhs);
            const auto rhs = static_cast<std::size_t>(pair.rhs);

            constexpr std::size_t offset = sizeof(std::size_t) * 4 / 2;
            return (lhs << offset) ^ rhs;
        }
    };
}  // namespace Z::Zaban::Langs::ZLang
