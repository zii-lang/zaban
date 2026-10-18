#include <Z/Zaban/AST/Annotations/ArrayAnnotation.hpp>
#include <Z/Zaban/AST/Annotations/PointerAnnotation.hpp>
#include <Z/Zaban/AST/Annotations/PrimitiveAnnotation.hpp>
#include <Z/Zaban/AST/Atomics/Identifier.hpp>
#include <Z/Zaban/Langs/CLang/AST/Annotations/PrimitiveAnnotation.hpp>
#include <Z/Zaban/Langs/CLang/AST/Expressions/InitializerListExpression.hpp>
#include <Z/Zaban/Langs/CLang/Parser.hpp>
#include <Z/Zaban/Langs/CLang/ParserDiagnostic.hpp>
#include <cstddef>
#include <optional>

#include "Z/Zaban/AST/Declarations/LetDeclaration.hpp"
#include "Z/Zaban/AST/Expressions/FunctionExpression.hpp"

namespace Z::Zaban::Langs::CLang {

    CDeclaration CParser::parse_globals() const {
        return this->parse_declaration(true);
    }

    // return_type function_name(parameters);
    // function name is already wrapped by the LetDeclaration and
    // can be accessed through that later on
    CExpression CParser::parse_function(CAnnotation ret_type,
                                        bool        allow_body) const {
        // NOTE: this method is already being called when a
        // '(' is seen
        auto start  = this->advance();
        auto params = this->parse_parameters();
        if (!params.has_value()) return nullptr;
        auto end = this->expect(CTokenKind::RParen,
                                CParserDiagnosticKind::ErrorExpectedToken);
        if (!end) return nullptr;

        Zaban::AST::Statement<COffsetType> block = nullptr;
        if (this->check(CTokenKind::LBrace)) {
            if (!allow_body) {
                this->report(CParserDiagnosticKind::ErrorDeclarationNotAllowed);
                return nullptr;
            }
            block = this->parse_block_statement();
            if (!block) return nullptr;
        } else {
            if (!this->expect(CTokenKind::Semicolon,
                              CParserDiagnosticKind::ErrorExpectedToken)) {
                return nullptr;
            }
        }

        auto range = span(start->range, block ? block->location() : end->range);

        return make_node<Zaban::AST::FunctionExpressionNode<COffsetType>>(
            range, std::move(params.value()), std::move(ret_type),
            std::move(block));
    }
    // TODO: only one declarator per declaration. `int a, b;` reports
    // "expected token" at ','. supporting it needs '*' to move from
    // parse_types() to the declarator (in `int *a, b;` only a is a
    // pointer) and a return type that can hold several declarations.
    // TODO: storage classes (static, extern) have no place in
    // LetDeclaration. add a C declaration node when they are parsed.
    CDeclaration CParser::parse_declaration(bool allow_body) const {
        // type
        auto base = this->parse_types();
        if (!base) return nullptr;
        auto start_offset = base->location();

        // identifier
        auto name =
            this->expect(CTokenKind::Identifier,
                         CParserDiagnosticKind::ErrorExpectedIdentifier);
        if (!name) return nullptr;
        Zaban::AST::Atomic<COffsetType> atom =
            make_node<Zaban::AST::Atomics::Identifier<COffsetType>>(
                name->range);

        // check to see if its a function prototype
        if (this->check(CTokenKind::LParen)) {
            auto function = this->parse_function(std::move(base), allow_body);
            if (!function) return nullptr;
            auto range = span(start_offset, this->previous()->range);
            return make_node<
                Zaban::AST::Declarations::LetDeclaration<COffsetType>>(
                range, std::move(atom), std::move(function));
        }

        auto var_type = this->parse_array_suffix(std::move(base));
        if (!var_type) return nullptr;

        // if it has an initializer
        CExpression initializer = nullptr;
        if (this->check(CTokenKind::Equal)) {
            this->advance();
            initializer = this->parse_initializer();
            if (!initializer) return nullptr;
        }

        auto end = this->expect(CTokenKind::Semicolon,
                                CParserDiagnosticKind::ErrorExpectedToken);
        if (!end) return nullptr;
        auto range = span(start_offset, end->range);

        return make_node<Zaban::AST::Declarations::LetDeclaration<COffsetType>>(
            range, std::move(atom), std::move(var_type),
            std::move(initializer));
    }

    // base-type '*'*
    // TODO: struct/union/enum are not handled yet. each needs its own
    // path: a tag name and/or a '{ fields }' body, building a
    // StructAnnotation / VariantAnnotation / EnumAnnotation. until then
    // they report ErrorExpectedType, which is misleading since they are
    // types; a dedicated "not supported yet" diagnostic would be
    // clearer.
    // TODO: qualifiers (const, volatile) are not parsed. decide whether
    // they live on the annotation (they apply per pointer level: `int
    // *const p`). NOTE: specifiers are only collected here. whether the
    // combination is valid (C11 6.7.2p2) is checked in semantic
    // analysis.
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
    // TODO: each ArrayAnnotation's range covers only its own '[ size
    // ]', not the element type. decide if the outer node should span
    // the whole declarator. NOTE: whether a size is a constant
    // expression (ErrorArraySizeNotConstant) is left to semantic
    // analysis; the parser only checks the syntax.
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
    // parameter-list between '(' and ')'. neither paren is consumed here,
    // parse_function() owns them. std::nullopt means failed
    // an empty vector means () or (void).
    std::optional<std::vector<CParameter>> CParser::parse_parameters() const {
        using ParameterNode = Zaban::AST::ParameterNode<COffsetType>;
        using Identifier    = Zaban::AST::Atomics::Identifier<COffsetType>;
        std::vector<CParameter> params{};

        if (this->check(CTokenKind::RParen)) return params;

        // (void) -> no parameters. only `void` directly followed by ')'
        // (void *p) is a real parameter
        if (const auto* next = this->peek(1);
            this->check(CTokenKind::Void) && next &&
            next->kind == CTokenKind::RParen) {
            this->advance();
            return params;
        }

        while (true) {
            // '...' must be last and needs a parameter before it.
            if (this->check(CTokenKind::Ellipsis)) {
                if (params.empty()) {
                    this->report(CParserDiagnosticKind::ErrorElipsisComesLast);
                    return std::nullopt;
                }
                const auto* ellipsis = this->advance();
                params.push_back(make_node<ParameterNode>(
                    ellipsis->range, Zaban::AST::Atomic<COffsetType>{}, true));
                break;
            }
            auto type_ = this->parse_types();
            if (!type_) return std::nullopt;
            auto start_offset = type_->location();

            // the name is optional in prototypes such as int f(int, char*).
            // an unnamed parameter stores a nullptr name.
            Zaban::AST::Atomic<COffsetType> name = nullptr;
            if (this->check(CTokenKind::Identifier)) {
                name = make_node<Identifier>(this->advance()->range);
            }
            auto param_type = this->parse_array_suffix(std::move(type_));
            if (!param_type) return std::nullopt;
            auto range = span(start_offset, this->previous()->range);
            params.push_back(make_node<ParameterNode>(range, std::move(name),
                                                      std::move(param_type)));
            if (!this->check(CTokenKind::Comma)) {
                break;
            }
            this->advance();
        }
        return params;
    }

    // TODO: designated initializers (`.x = 1`, `[0] = 1`) are not
    // parsed.
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
