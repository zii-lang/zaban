#pragma once

#include <concepts>
#include <cstdint>
#include <type_traits>

namespace Z::Zaban {

    /**
     * Marks an enum as a bitmask enum.
     *
     * By default, enums do not support bitwise operations.
     * Specialize this trait for enums that represent flags:
     *
     *     template <>
     *     struct enable_bitmask_operators<MyFlags> : std::true_type {};
     */
    template<typename E>
    struct enable_bitmask_operators : std::false_type {};

    /**
     * Concept for enums explicitly enabled as bitmasks.
     */
    template<typename E>
    concept BitmaskEnum =
        std::is_enum_v<E> && enable_bitmask_operators<E>::value;

    /**
     * Convert a bitmask enum to its underlying integer type.
     */
    template<BitmaskEnum E>
    [[nodiscard]]
    constexpr auto to_underlying(E value) noexcept
        -> std::underlying_type_t<E> {
        return static_cast<std::underlying_type_t<E>>(value);
    }

    // -----------------------------------------------------------------------------
    // Bitwise operators
    // -----------------------------------------------------------------------------

    template<BitmaskEnum E>
    [[nodiscard]]
    constexpr E operator|(E lhs, E rhs) noexcept {
        return static_cast<E>(to_underlying(lhs) | to_underlying(rhs));
    }

    template<BitmaskEnum E>
    [[nodiscard]]
    constexpr E operator&(E lhs, E rhs) noexcept {
        return static_cast<E>(to_underlying(lhs) & to_underlying(rhs));
    }

    template<BitmaskEnum E>
    [[nodiscard]]
    constexpr E operator^(E lhs, E rhs) noexcept {
        return static_cast<E>(to_underlying(lhs) ^ to_underlying(rhs));
    }

    template<BitmaskEnum E>
    [[nodiscard]]
    constexpr E operator~(E value) noexcept {
        return static_cast<E>(~to_underlying(value));
    }

    // -----------------------------------------------------------------------------
    // Assignment operators
    // -----------------------------------------------------------------------------

    template<BitmaskEnum E>
    constexpr E& operator|=(E& lhs, E rhs) noexcept {
        lhs = lhs | rhs;
        return lhs;
    }

    template<BitmaskEnum E>
    constexpr E& operator&=(E& lhs, E rhs) noexcept {
        lhs = lhs & rhs;
        return lhs;
    }

    template<BitmaskEnum E>
    constexpr E& operator^=(E& lhs, E rhs) noexcept {
        lhs = lhs ^ rhs;
        return lhs;
    }

    // -----------------------------------------------------------------------------
    // Queries
    // -----------------------------------------------------------------------------

    /**
     * Returns true if at least one bit is set.
     *
     *     any(flags)
     */
    template<BitmaskEnum E>
    [[nodiscard]]
    constexpr bool any(E value) noexcept {
        return to_underlying(value) != 0;
    }

    /**
     * Returns true if no bits are set.
     *
     *     none(flags)
     */
    template<BitmaskEnum E>
    [[nodiscard]]
    constexpr bool none(E value) noexcept {
        return to_underlying(value) == 0;
    }

    /**
     * Returns true if all bits in `flags` are present in `value`.
     *
     *     has(flags, MyFlags::Foo | MyFlags::Bar)
     */
    template<BitmaskEnum E>
    [[nodiscard]]
    constexpr bool has(E value, E flags) noexcept {
        return (value & flags) == flags;
    }

    /**
     * Returns true if any bit in `flags` is present in `value`.
     *
     *     has_any(flags, MyFlags::Foo | MyFlags::Bar)
     */
    template<BitmaskEnum E>
    [[nodiscard]]
    constexpr bool has_any(E value, E flags) noexcept {
        return any(value & flags);
    }

    /**
     * Returns true if none of the bits in `flags` are present in `value`.
     */
    template<BitmaskEnum E>
    [[nodiscard]]
    constexpr bool has_none(E value, E flags) noexcept {
        return none(value & flags);
    }

    /**
     * Returns true if `value` contains exactly the specified flags.
     */
    template<BitmaskEnum E>
    [[nodiscard]]
    constexpr bool has_exact(E value, E flags) noexcept {
        return value == flags;
    }

    // -----------------------------------------------------------------------------
    // Flag manipulation helpers
    // -----------------------------------------------------------------------------

    /**
     * Returns `value` with `flags` added.
     *
     *     flags = set(flags, MyFlags::Foo);
     */
    template<BitmaskEnum E>
    [[nodiscard]]
    constexpr E set(E value, E flags) noexcept {
        return value | flags;
    }

    /**
     * Returns `value` with `flags` removed.
     *
     *     flags = unset(flags, MyFlags::Foo);
     */
    template<BitmaskEnum E>
    [[nodiscard]]
    constexpr E unset(E value, E flags) noexcept {
        return value & ~flags;
    }

    /**
     * Returns `value` with `flags` toggled.
     *
     *     flags = toggle(flags, MyFlags::Foo);
     */
    template<BitmaskEnum E>
    [[nodiscard]]
    constexpr E toggle(E value, E flags) noexcept {
        return value ^ flags;
    }

    /**
     * Returns `value` with only `flags` retained.
     */
    template<BitmaskEnum E>
    [[nodiscard]]
    constexpr E mask(E value, E flags) noexcept {
        return value & flags;
    }

}  // namespace Z::Zaban

// -----------------------------------------------------------------------------
// Opt-in macro
// -----------------------------------------------------------------------------
//
// IMPORTANT:
// This macro must be invoked inside namespace Z::Zaban because explicit
// template specializations must be declared in the namespace containing
// the primary template.
//
// Example:
//
// namespace Z::Zaban {
//     Z_ENABLE_BITMASK_OPERATORS(
//         Langs::ZLang::ZLexerInvalidationFlag
//     );
// }
//
#define Z_ENABLE_BITMASK_OPERATORS(Enum) \
    template<>                           \
    struct enable_bitmask_operators<Enum> : std::true_type {}
