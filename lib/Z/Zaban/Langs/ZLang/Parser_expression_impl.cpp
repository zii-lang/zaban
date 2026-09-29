#include <Z/Zaban/AST/Expression.hpp>
#include <Z/Zaban/AST/Expressions/AssignmentExpression.hpp>
#include <Z/Zaban/Langs/ZLang/Parser.hpp>

namespace Z::Zaban::Langs::ZLang {
    AST::Expression<ZOffsetType> ZParser::parse_expression() const {
        auto expr = this->parse_assignment();
        if (expr == nullptr) {
            // TODO: report error.
        }
        return expr;
    }

    AST::Expression<ZOffsetType> ZParser::parse_assignment() const {
        return nullptr;
    }

    AST::Expression<ZOffsetType> ZParser::parse_logical_or() const {
        return nullptr;
    }

    AST::Expression<ZOffsetType> ZParser::parse_logical_and() const {
        return nullptr;
    }

    AST::Expression<ZOffsetType> ZParser::parse_bitwise_or() const {
        return nullptr;
    }

    AST::Expression<ZOffsetType> ZParser::parse_bitwise_xor() const {
        return nullptr;
    }

    AST::Expression<ZOffsetType> ZParser::parse_bitwise_and() const {
        return nullptr;
    }

    AST::Expression<ZOffsetType> ZParser::parse_equality() const {
        return nullptr;
    }

    AST::Expression<ZOffsetType> ZParser::parse_comparison() const {
        return nullptr;
    }

    AST::Expression<ZOffsetType> ZParser::parse_shifting() const {
        return nullptr;
    }

    AST::Expression<ZOffsetType> ZParser::parse_additive() const {
        return nullptr;
    }

    AST::Expression<ZOffsetType> ZParser::parse_multipicative() const {
        return nullptr;
    }

    AST::Expression<ZOffsetType> ZParser::parse_unary() const {
        return nullptr;
    }

    AST::Expression<ZOffsetType> ZParser::parse_suffix() const {
        return nullptr;
    }

}  // namespace Z::Zaban::Langs::ZLang
