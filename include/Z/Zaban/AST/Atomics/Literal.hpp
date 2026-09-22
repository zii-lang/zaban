#pragma once

#include <Z/Zaban/AST/Atomic.hpp>
#include <Z/Zaban/AST/Parameter.hpp>
#include <variant>
#include <vector>

namespace Z::Zaban::AST::Atomics {
    template<typename OffsetType = std::size_t>
    class LiterlNode;

    template<typename OffsetType = std::size_t>
    using Literal = std::shared_ptr<LiteralNode>;
    /** @brief Represents the underlying value stored by a literal node.
     *
     * LiteralValue stores the possible compile-time values that can be
     * represented by a literal expression.
     *
     * The first alternative represents a null literal. Other alternatives
     * represent primitive values and aggregate literal values.
     */
    template<typename OffsetType = std::size_t>
    using LiteralValue = std::variant<std::monostate,  // null literal
                                      bool,            // boolean literal
                                      std::string,  // numeric, string, or named
                                                    // literal value
                                      std::vector<Node>,      // array literal
                                      std::vector<Parameter>  // struct literal
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
    class LiteralNode : public Atomic {
       private:
        // The specific literal category.
        const LiteralKind kind;

        // The underlying literal data.
        const LiteralValue value;

       public:
        /** @brief Creates a literal with a specific kind and value. */
        LiteralNode(LiteralKind kind, LiteralValue value) :
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

        /** @brief Returns the underlying literal value. */
        LiteralValue get_value() {
            return this->value;
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

        /** @brief Returns the string representation of numeric or string
         * literals. */
        std::string get_string() {
            switch (this->kind) {
                case LiteralKind::Numeric:
                case LiteralKind::String:
                    return std::get<std::string>(this->value);
                default:
                    return nullptr;
            }
        }

        /** @brief Returns the fields of a structure literal. */
        std::vector<Parameter> get_struct() {
            return std::get<std::vector<Parameter>>(this->value);
        }

        /** @brief Destroys the literal node. */
        ~LiteralNode() = default;
    };
}  // namespace Z::Zaban::AST::Atomics
