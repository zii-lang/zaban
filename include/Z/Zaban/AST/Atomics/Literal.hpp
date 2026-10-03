#pragma once

#include <Z/Zaban/AST/Atomic.hpp>
#include <Z/Zaban/AST/Parameter.hpp>
#include <memory>
#include <variant>
#include <vector>

namespace Z::Zaban::AST::Atomics {
    enum class LiteralKind {
        Null,
        Boolean,
        Numeric,
        String,

        Array,
        Struct,
        Variant,
    };

    /** @brief Represents the underlying value stored by a literal node.
     *
     * LiteralValue stores the possible compile-time values that can be
     * represented by a literal expression.
     *
     * The first alternative represents a null literal. Other alternatives
     * represent primitive values and aggregate literal values.
     */
    template<typename OffsetType = std::size_t>
    using LiteralValue =
        std::variant<std::monostate,  // null numeric, string, literal
                     bool,            // boolean literal
                     std::vector<std::shared_ptr<Node<OffsetType>>>,  // array
                                                                      // literal
                     std::vector<Parameter<OffsetType>>  // struct literal
                     >;
    /** @brief Represents a literal value in the AST.
     *
     * ILiteral stores a compile-time constant value together with its literal
     * category. Literals represent values that can be directly written in
     * source code, such as numbers, strings, booleans, arrays, structures, and
     * variants.
     *
     * The stored value is kept in a LiteralValue variant and can be accessed
     * through typed getters based on the literal kind.
     */
    template<typename OffsetType = std::size_t>
    class Literal : public AtomicNode<OffsetType> {
       private:
        // The specific literal category.
        mutable LiteralKind kind = LiteralKind::Null;

        // The underlying literal data.
        mutable LiteralValue<OffsetType> value = std::monostate{};

       public:
        Literal() = default;
        /** @brief Creates a literal with a specific kind. */
        Literal(LiteralKind kind) : kind(kind) {};
        /** @brief Creates a literal with a specific kind and value. */
        Literal(LiteralKind kind, LiteralValue<OffsetType> value) :
            kind(kind), value(std::move(value)) {
        }

        /** @brief Returns the primary expression category. */
        AtomicKind get_atomic_kind() const override {
            return AtomicKind::Literal;
        }

        /** @brief Returns the literal category. */
        LiteralKind get_kind() const {
            return this->kind;
        }

        void set_kind(LiteralKind kind) const {
            this->kind = kind;
        }

        /** @brief Returns the underlying literal value. */
        LiteralValue<OffsetType> get_value() {
            return this->value;
        }

        void set_value(LiteralValue<OffsetType> value) const {
            this->value = value;
        }

        /** @brief Returns the stored value as the requested type. */
        template<typename T>
        T get() {
            return std::get<T>(this->value);
        }

        /** @brief Returns the boolean value if this is a boolean literal. */
        bool get_bool() {
            if (this->kind == LiteralKind::Boolean) {
                return std::get<bool>(this->value);
            }
            return false;
        }

        /** @brief Returns the fields of a structure literal. */
        std::vector<Parameter<OffsetType>> get_struct() {
            return std::get<std::vector<Parameter<OffsetType>>>(this->value);
        }

        Atomic<OffsetType> get_ptr() {
            return std::make_shared<Literal<OffsetType>>(*this);
        }

        /** @brief Destroys the literal node. */
        ~Literal() = default;
    };
}  // namespace Z::Zaban::AST::Atomics
