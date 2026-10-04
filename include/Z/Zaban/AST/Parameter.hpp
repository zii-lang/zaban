#pragma once

#include <Z/Zaban/AST/Annotation.hpp>
#include <Z/Zaban/AST/Atomic.hpp>
#include <Z/Zaban/AST/Atomics/Identifier.hpp>
#include <Z/Zaban/AST/Expression.hpp>
#include <Z/Zaban/AST/Node.hpp>
#include <memory>
#include <string>

namespace Z::Zaban::AST {
    /** @brief Represents a named parameter declaration.
     *
     * ParameterNode stores the information required to describe a parameter,
     * including its name, optional type annotation, optional initializer, and
     * variadic state.
     *
     * Parameters can be used for function arguments, structure fields, and
     * other language constructs that introduce named values.
     */
    template<typename OffsetType = std::size_t>
    class ParameterNode : public Node<OffsetType> {
       private:
        Atomic<OffsetType> _name;

        // Optional type annotation.
        Annotation<OffsetType> _annotation = nullptr;

        // Optional default initializer.
        Expression<OffsetType> _initializer = nullptr;

        // Whether this parameter accepts variadic arguments.
        bool _is_vararg = false;

       public:
        /** @brief Creates a parameter with only a name. */
        ParameterNode(Atomic<OffsetType> name) : _name(name) {
        }

        /** @brief Creates a parameter with a type annotation. */
        ParameterNode(Atomic<OffsetType>     name,
                      Annotation<OffsetType> annotation) :
            _name(name), _annotation(annotation) {
        }

        /** @brief Creates a parameter with a default initializer expression. */
        ParameterNode(Atomic<OffsetType>     name,
                      Expression<OffsetType> initializer) :
            _name(name), _initializer(initializer) {
        }

        /** @brief Creates a parameter with a type annotation and initializer.
         */
        ParameterNode(Atomic<OffsetType>     name,
                      Annotation<OffsetType> annotation,
                      Expression<OffsetType> initializer) :
            _name(name), _annotation(annotation), _initializer(initializer) {
        }

        /** @brief Creates a variadic parameter. */
        ParameterNode(Atomic<OffsetType> name, bool is_vararg = true) :
            _name(name), _is_vararg(is_vararg) {
        }

        /** @brief Returns the parameter name. */
        Atomic<OffsetType> get_name() const {
            return _name;
        }

        /** @brief Returns whether this parameter is variadic. */
        bool is_vararg() const {
            return _is_vararg;
        }

        /** @brief Returns the parameter type annotation, if available. */
        Annotation<OffsetType> get_annotation() const {
            return _annotation;
        }

        /** @brief Returns the initializer expression, if available. */
        Expression<OffsetType> get_initializer() const {
            return _initializer;
        }

        const NodeKind node_kind() const override {
            return NodeKind::Parameter;
        }
    };

    /** @brief Shared reference to a parameter declaration.
     *
     * Parameter provides shared ownership semantics for ParameterBase nodes
     * in the AST. Parameters represent named values with optional type
     * annotations, initializers, or variadic behavior.
     */
    template<typename OffsetType = std::size_t>
    using Parameter = std::shared_ptr<ParameterNode<OffsetType>>;
}  // namespace Z::Zaban::AST
