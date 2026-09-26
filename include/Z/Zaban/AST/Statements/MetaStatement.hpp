#pragma once

#include <Z/Zaban/AST/Statement.hpp>

namespace Z::Zaban::AST::Statements {
    /** @brief Represents a metadata statement.
     *
     * MetaStatement stores compiler or language metadata attached to a
     * statement. Metadata may optionally wrap an inner statement that it
     * modifies.
     */
    template<typename OffsetType = std::size_t>
    class MetaStatement : public StatementNode {
       private:
        const std::string _name;
        const Statement   _inner = nullptr;

       public:
        /** @brief Creates metadata without an attached statement. */
        MetaStatement(std::string name) : _name(std::move(name)) {
        }

        /** @brief Creates metadata wrapping another statement. */
        MetaStatement(std::string name, Statement inner) :
            _name(std::move(name)), _inner(std::move(inner)) {
        }

        StatementKind statement_kind() const override {
            return StatementKind::Meta;
        }

        /** @brief Returns the metadata name. */
        const std::string& get_name() const {
            return _name;
        }

        /** @brief Returns whether this metadata has an inner statement. */
        bool has_inner() const {
            return _inner != nullptr;
        }

        /** @brief Returns the wrapped statement, if available. */
        Statement get_inner() const {
            return _inner;
        }
    };
}  // namespace Z::Zaban::AST::Statements
