#include <Z/Zaban/AST/Annotations/PointerAnnotation.hpp>
#include <Z/Zaban/AST/Annotations/PrimitiveAnnotation.hpp>
#include <Z/Zaban/AST/Atomics/Identifier.hpp>
#include <Z/Zaban/Langs/CLang/AST/Expressions/InitializerListExpression.hpp>
#include <Z/Zaban/Langs/CLang/Parser.hpp>

#include "Z/Zaban/AST/Annotations/ArrayAnnotation.hpp"
#include "Z/Zaban/AST/Expression.hpp"
#include "Z/Zaban/Langs/CLang/AST/Annotations/PrimitiveAnnotation.hpp"
#include "Z/Zaban/Langs/CLang/ParserDiagnostic.hpp"

namespace Z::Zaban::Langs::CLang {
    CDeclaration CParser::parse_globals() const {
    }
    CDeclaration CParser::parse_declaration() const {
    }

    // base-type '*'*
    // TODO: struct/union/enum are not handled yet.
    CAnnotation CParser::parse_types() const {
        using AST::Annotations::TypeSpecifier;
        using Zaban::AST::Annotations::PointerAnnotation;
        using Zaban::Langs::CLang::AST::Annotations::PrimitiveAnnotationNode;
        std::vector<TypeSpecifier<COffsetType>> specifiers{};

        for (const auto* current = this->peek();
             current && is_type_specifier(current->kind);) {
            if (current->kind == CTokenKind::Struct ||
                current->kind == CTokenKind::Union ||
                current->kind == CTokenKind::Enum) {
                this->report(CParserDiagnosticKind::ErrorExpectedType);
                return nullptr;
            }
            specifiers.push_back(TypeSpecifier{current->kind, current->range});

            this->advance();
            current = this->peek();
        }
        if (specifiers.empty()) {
            report(CParserDiagnosticKind::ErrorExpectedType);
            return nullptr;
        }
        auto        range = span(specifiers[0].range, specifiers.back().range);
        CAnnotation node  = make_node<PrimitiveAnnotationNode<COffsetType>>(
            range, std::move(specifiers));
        while (this->check(CTokenKind::Asterisk)) {
            auto star  = this->advance();
            auto range = span(node->location(), star->range);
            node = make_node<PointerAnnotation<COffsetType>>(range,
                                                             std::move(node));
        }
        return node;
    }

    /// parses int a[2][3]
    CAnnotation CParser::parse_array_suffix(CAnnotation base) const {
        using Zaban::AST::Annotations::ArrayAnnotation;
        // one '[ size? ]', size is nullptr when empty (int a[]).
        struct Dimension {
            CExpression              size;
            OffsetRange<COffsetType> range;
        };

        std::vector<Dimension> dimensions{};
        while (this->check(CTokenKind::LBrak)) {
            const auto* lbrak = this->advance();

            CExpression size = nullptr;
            if (!this->check(CTokenKind::RBrak)) {
                size = this->parse_expression();
                if (!size) return nullptr;
            }

            const auto* rbrak = this->expect(
                CTokenKind::RBrak, CParserDiagnosticKind::ErrorExpectedToken);
            if (!rbrak) return nullptr;

            dimensions.push_back(
                {std::move(size), span(lbrak->range, rbrak->range)});
        }

        for (auto it = dimensions.rbegin(); it != dimensions.rend(); ++it) {
            base = make_node<ArrayAnnotation<COffsetType>>(
                it->range, std::move(base), std::move(it->size));
        }
        return base;
    }
    std::vector<CParameter> CParser::parse_parameters() const {
    }
    CExpression CParser::parse_initializer() const {
        // int a = some expr
        if (!this->check(CTokenKind::LBrace)) {
            return this->parse_assignment();
        }
        // int a[] = a list initializer
        auto start = this->advance();

        std::vector<CExpression> elements{};
        while (!this->check(CTokenKind::RBrace)) {
            auto current = this->parse_initializer();
            if (!current) return nullptr;
            elements.push_back(std::move(current));
            if (this->check(CTokenKind::Comma))
                this->advance();
            else
                break;
        }
        auto end = this->expect(CTokenKind::RBrace,
                                CParserDiagnosticKind::ErrorExpectedToken);
        if (!end) return nullptr;
        auto range = span(start->range, end->range);

        using Zaban::Langs::CLang::AST::Expressions::
            InitializerListExpressionNode;
        return make_node<InitializerListExpressionNode<COffsetType>>(
            range, std::move(elements));
    }
}  // namespace Z::Zaban::Langs::CLang
