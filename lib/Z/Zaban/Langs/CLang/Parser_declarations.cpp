#include <Z/Zaban/AST/Annotation.hpp>
#include <Z/Zaban/AST/Annotations/ArrayAnnotation.hpp>
#include <Z/Zaban/AST/Annotations/PointerAnnotation.hpp>
#include <Z/Zaban/AST/Annotations/PrimitiveAnnotation.hpp>
#include <Z/Zaban/AST/Atomics/Identifier.hpp>
#include <Z/Zaban/AST/Declarations/LetDeclaration.hpp>
#include <Z/Zaban/AST/Expressions/FunctionExpression.hpp>
#include <Z/Zaban/AST/Parameter.hpp>
#include <Z/Zaban/Langs/CLang/AST/Annotations/ArrayAnnotation.hpp>
#include <Z/Zaban/Langs/CLang/AST/Annotations/EnumAnnotation.hpp>
#include <Z/Zaban/Langs/CLang/AST/Annotations/FunctionAnnotation.hpp>
#include <Z/Zaban/Langs/CLang/AST/Annotations/PrimitiveAnnotation.hpp>
#include <Z/Zaban/Langs/CLang/AST/Annotations/QualifierAnnotation.hpp>
#include <Z/Zaban/Langs/CLang/AST/Annotations/StructAnnotation.hpp>
#include <Z/Zaban/Langs/CLang/AST/Annotations/UnionAnnotation.hpp>
#include <Z/Zaban/Langs/CLang/AST/Declaration/CDeclaration.hpp>
#include <Z/Zaban/Langs/CLang/AST/Declaration/DeclarationGroup.hpp>
#include <Z/Zaban/Langs/CLang/AST/Declaration/EnumDeclaration.hpp>
#include <Z/Zaban/Langs/CLang/AST/Declaration/StructDeclaration.hpp>
#include <Z/Zaban/Langs/CLang/AST/Declaration/UnionDeclaration.hpp>
#include <Z/Zaban/Langs/CLang/AST/Expressions/InitializerListExpression.hpp>
#include <Z/Zaban/Langs/CLang/Parser.hpp>
#include <Z/Zaban/Langs/CLang/ParserDiagnostic.hpp>
#include <Z/Zaban/Langs/CLang/TokenKind.hpp>
#include <algorithm>
#include <optional>

namespace Z::Zaban::Langs::CLang {

    using UnionNode =
        Langs::CLang::AST::Annotations::UnionAnnotationNode<COffsetType>;
    using UnionDeclaration =
        Langs::CLang::AST::Declaration::UnionDeclarationNode<COffsetType>;
    using EnumNode =
        Langs::CLang::AST::Annotations::EnumAnnotationNode<COffsetType>;
    using EnumDeclaration =
        Langs::CLang::AST::Declaration::EnumDeclarationNode<COffsetType>;
    using StructNode =
        Langs::CLang::AST::Annotations::StructAnnotationNode<COffsetType>;
    using StructDeclaration =
        Langs::CLang::AST::Declaration::StructDeclarationNode<COffsetType>;
    using QualifierNode =
        Langs::CLang::AST::Annotations::QualifierAnnotationNode<COffsetType>;
    using Identifier    = Zaban::AST::Atomics::Identifier<COffsetType>;
    using ParameterNode = Zaban::AST::ParameterNode<COffsetType>;

    CDeclaration CParser::parse_globals() const {
        return this->parse_declaration(true);
    }

    bool CParser::starts_grouped_declarator(DeclaratorNameMode mode) const {
        if (!this->check(CTokenKind::LParen)) return false;

        // A named declarator must start with an identifier or grouping.
        // Its function suffix can only occur after that direct part.
        if (mode == DeclaratorNameMode::Required) return true;

        const auto* next = this->peek(1);
        if (!next) return false;

        switch (next->kind) {
            case CTokenKind::Asterisk:
            case CTokenKind::LParen:
            case CTokenKind::LBrak:
                return true;
            case CTokenKind::Identifier:
                return mode == DeclaratorNameMode::Optional;
            default:
                // A type specifier or ')' begins a function suffix instead.
                // Typedef names will require scoped name lookup here.
                return false;
        }
    }

    // A declarator has three parts, left to right:
    //
    //     *const *      name        [3](int)
    //     ^ prefix      ^ direct    ^ suffixes
    //
    // e.g. in `int *const p[3]` the prefix is `*const`, the direct part is
    // `p`, and the suffix is `[3]`
    std::optional<CParser::ParsedDeclarator> CParser::parse_declarator(
        DeclaratorNameMode mode) const {
        const auto* start = this->peek();
        if (!start || this->m_stream.end()) {
            this->report(CParserDiagnosticKind::ErrorUnexpectedEndOfFile);
            return std::nullopt;
        }

        auto pointers = this->parse_pointer_prefix();

        auto result = this->parse_direct_declarator(mode);
        if (!result) return std::nullopt;

        while (this->check(CTokenKind::LBrak) ||
               this->check(CTokenKind::LParen)) {
            auto suffix = this->check(CTokenKind::LParen)
                              ? this->parse_function_suffix()
                              : this->parse_array_suffix();
            if (!suffix) return std::nullopt;
            result->operations.push_back(std::move(*suffix));
        }

        // Prefix stars are farther from the identifier than this lvl's
        // suffixes. we should reverse their source order when appending them.
        for (auto it = pointers.rbegin(); it != pointers.rend(); ++it) {
            result->operations.push_back(std::move(*it));
        }

        if (this->peek() != start) {
            result->range = span(start->range, this->previous()->range);
        }
        return result;
    }

    // `* const *` -> one Pointer op per star, in source order. qualifiers
    // after a '*' apply to that pointer: in `int *const p`, p itself can't
    // change, the int can.
    std::vector<CParser::DeclaratorOp> CParser::parse_pointer_prefix() const {
        std::vector<DeclaratorOp> pointers;
        while (this->check(CTokenKind::Asterisk)) {
            const auto* star = this->advance();

            std::vector<AST::Annotations::TypeQualifier<COffsetType>>
                qualifiers;
            while (const auto* tok = this->peek()) {
                if (!this->is_type_qualifier(tok->kind)) break;
                qualifiers.push_back({tok->kind, tok->range});
                this->advance();
            }

            pointers.push_back({
                DeclaratorOpKind::Pointer,
                span(star->range, this->previous()->range),
                nullptr,
                {},
                std::move(qualifiers),
                false,
                false,
            });
        }
        return pointers;
    }

    // The middle of a declarator: a name (`p`), a parenthesized declarator
    // (`(*p)`), or, for abstract declarators, nothing at all.
    std::optional<CParser::ParsedDeclarator> CParser::parse_direct_declarator(
        DeclaratorNameMode mode) const {
        ParsedDeclarator result{
            nullptr,
            {},
            this->peek()->range,
        };

        if (mode != DeclaratorNameMode::Forbidden &&
            this->check(CTokenKind::Identifier)) {
            const auto* identifier = this->advance();
            result.name            = make_node<Identifier>(identifier->range);
            return result;
        }

        if (this->starts_grouped_declarator(mode)) {
            this->advance();  // (
            const auto* nested_start = this->peek();
            auto        nested       = this->parse_declarator(mode);
            if (!nested) return std::nullopt;
            if (this->peek() == nested_start) {
                this->report(CParserDiagnosticKind::ErrorExpectedIdentifier);
                return std::nullopt;
            }
            if (!this->expect(CTokenKind::RParen,
                              CParserDiagnosticKind::ErrorExpectedToken)) {
                return std::nullopt;
            }
            return nested;
        }

        if (mode == DeclaratorNameMode::Required) {
            this->report(CParserDiagnosticKind::ErrorExpectedIdentifier);
            return std::nullopt;
        }
        return result;  // abstract: no name
    }

    // `( parameters )`
    std::optional<CParser::DeclaratorOp> CParser::parse_function_suffix()
        const {
        const auto* opening    = this->advance();  // (
        auto        parameters = this->parse_parameters();
        if (!parameters) return std::nullopt;
        const auto* closing = this->expect(
            CTokenKind::RParen, CParserDiagnosticKind::ErrorExpectedToken);
        if (!closing) return std::nullopt;
        return DeclaratorOp{
            DeclaratorOpKind::Function,
            span(opening->range, closing->range),
            nullptr,
            std::move(*parameters),
            {},
            false,
            false,
        };
    }

    // `[ size ]` or `[]`. parameters may also write `[const 3]`,
    // `[static 3]`, `[const static 3]`, `[static const 3]` and `[*]`. those
    // are always parsed here. check_array_forms() rejects them where they
    // aren't allowed
    std::optional<CParser::DeclaratorOp> CParser::parse_array_suffix() const {
        const auto* opening = this->advance();  // [

        bool is_static      = false;
        bool is_unspecified = false;
        std::vector<AST::Annotations::TypeQualifier<COffsetType>> qualifiers;
        CExpression bound = nullptr;

        // static can come before the qualifiers!
        if (this->check(CTokenKind::Static)) {
            this->advance();
            is_static = true;
        }
        while (const auto* tok = this->peek()) {
            if (!this->is_type_qualifier(tok->kind)) break;
            qualifiers.push_back({tok->kind, tok->range});
            this->advance();
        }
        // it can also come after them but only once
        if (this->check(CTokenKind::Static)) {
            if (is_static) {  // [static const static 3]
                this->report(CParserDiagnosticKind::ErrorUnexpectedToken);
                return std::nullopt;
            }
            this->advance();
            is_static = true;
        }

        // `[*]` is an array of unknown size it must be `*` then `]`:
        // `[*p]` is an ordinary size that dereferences p.
        const auto* after_star = this->peek(1);
        if (this->check(CTokenKind::Asterisk) && after_star &&
            after_star->kind == CTokenKind::RBrak) {
            this->advance();  // *
            is_unspecified = true;
        } else if (!this->check(CTokenKind::RBrak)) {
            bound = this->parse_assignment();
            if (!bound) return std::nullopt;
        }

        // static states "at least N elements" so it needs an N
        if (is_static && !bound) {  // [static] and [static *]
            this->report(CParserDiagnosticKind::ErrorExpectedExpression);
            return std::nullopt;
        }

        const auto* closing = this->expect(
            CTokenKind::RBrak, CParserDiagnosticKind::ErrorExpectedToken);
        if (!closing) return std::nullopt;

        return DeclaratorOp{
            DeclaratorOpKind::Array, span(opening->range, closing->range),
            std::move(bound),        {},
            std::move(qualifiers),   is_static,
            is_unspecified,
        };
    }

    // `[const 3]`, `[static 3]` and `[*]` are only valid in a function
    // parameter, and only in the first brackets after the name. operations
    // are ordered outward from the name so thats operations[0]:
    //
    //   int a[const][4]  -> [Array(const), Array(4)]  const at 0: ok
    //   int a[4][const]  -> [Array(4), Array(const)]  const at 1: error
    //   int (*a)[const]  -> [Pointer, Array(const)]   const at 1: error
    bool CParser::check_array_forms(const ParsedDeclarator& declarator,
                                    bool is_parameter) const {
        const auto& ops = declarator.operations;
        for (std::size_t i = 0; i < ops.size(); ++i) {
            const auto& op = ops[i];
            const bool  uses_param_form =
                op.kind == DeclaratorOpKind::Array &&
                (!op.qualifiers.empty() || op.is_static || op.is_unspecified);
            if (!uses_param_form) continue;
            if (is_parameter && i == 0) continue;  // the one allowed spot
            this->report_at(CParserDiagnosticKind::ErrorDeclarationNotAllowed,
                            op.range);
            return false;
        }
        return true;
    }

    CAnnotation CParser::build_declarator_type(
        CAnnotation base, const ParsedDeclarator& declarator) const {
        using Zaban::AST::Annotations::ArrayAnnotation;
        using Zaban::AST::Annotations::PointerAnnotation;

        for (auto it = declarator.operations.rbegin();
             it != declarator.operations.rend(); ++it) {
            switch (it->kind) {
                case DeclaratorOpKind::Pointer: {
                    auto range = OffsetRange<COffsetType>(
                        std::min(base->location().begin, it->range.begin),
                        std::max(base->location().end, it->range.end));
                    base = make_node<PointerAnnotation<COffsetType>>(
                        range, std::move(base));
                    // `*const` qualifies the pointer, so it wraps the
                    // pointer node, not the pointee.
                    if (!it->qualifiers.empty()) {
                        auto qualifiers =
                            it->qualifiers;  // declarator is const
                        base = make_node<QualifierNode>(range, std::move(base),
                                                        std::move(qualifiers));
                    }
                    break;
                }
                case DeclaratorOpKind::Function: {
                    auto range = OffsetRange<COffsetType>(
                        std::min(base->location().begin, it->range.begin),
                        std::max(base->location().end, it->range.end));
                    base = make_node<
                        AST::Annotations::FunctionAnnotationNode<COffsetType>>(
                        range, std::move(base), it->parameters);
                    break;
                }
                case DeclaratorOpKind::Array: {
                    // plain `[3]` / `[]` keep the shared node. only the
                    // parameter forms need the C-specific one.
                    if (it->qualifiers.empty() && !it->is_static &&
                        !it->is_unspecified) {
                        base = make_node<ArrayAnnotation<COffsetType>>(
                            it->range, std::move(base), it->bound);
                        break;
                    }
                    auto qualifiers = it->qualifiers;  // declarator is const
                    base            = make_node<
                        AST::Annotations::CArrayAnnotationNode<COffsetType>>(
                        it->range, std::move(base), it->bound,
                        std::move(qualifiers), it->is_static,
                        it->is_unspecified);
                    break;
                }
            }
        }
        return base;
    }

    CDeclaration CParser::parse_declaration(bool allow_body) const {
        // type and storage classes
        auto specs = this->parse_declaration_specifiers();
        if (!specs.type) return nullptr;
        auto base         = specs.type;
        auto start_offset = specs.range;

        // Standalone type definitions or tag declarations. Enum definitions
        // may be unnamed because their enumerators introduce names.
        if (this->check(CTokenKind::Semicolon)) {
            // TODO: warn about the useless qualifier or storage class
            // (`static struct S;`), like gcc and clang do.
            CAnnotation tag_type = base;
            if (auto qualified = base->dyn_cast<QualifierNode>()) {
                tag_type = qualified->get_underlying();
            }
            auto struct_type = tag_type->dyn_cast<StructNode>();
            if (struct_type && struct_type->get_tag()) {
                auto semi  = this->advance();
                auto range = span(start_offset, semi->range);
                return make_node<StructDeclaration>(range,
                                                    std::move(struct_type));
            }
            auto enum_type = tag_type->dyn_cast<EnumNode>();
            if (enum_type &&
                (enum_type->get_tag() || enum_type->has_definition())) {
                auto semi = this->advance();
                return make_node<EnumDeclaration>(
                    span(start_offset, semi->range), std::move(enum_type));
            }
            auto union_type = tag_type->dyn_cast<UnionNode>();
            if (union_type && union_type->get_tag()) {
                auto semi  = this->advance();
                auto range = span(start_offset, semi->range);
                return make_node<UnionDeclaration>(range,
                                                   std::move(union_type));
            }
        }

        using Let = AST::Declaration::CLetDeclarationNode<COffsetType>;
        using FunctionType =
            AST::Annotations::FunctionAnnotationNode<COffsetType>;
        std::vector<CDeclaration> declarations;
        while (true) {
            auto declarator = this->parse_declarator();
            if (!declarator) return nullptr;
            if (!this->check_array_forms(*declarator, false)) return nullptr;
            auto type          = this->build_declarator_type(base, *declarator);
            auto function_type = std::dynamic_pointer_cast<FunctionType>(type);
            CExpression initializer = nullptr;
            auto        item_range  = declarator->range;

            if (this->check(CTokenKind::LBrace)) {
                if (!allow_body || !function_type || !declarations.empty()) {
                    this->report(
                        CParserDiagnosticKind::ErrorDeclarationNotAllowed);
                    return nullptr;
                }
                auto body = this->parse_block_statement();
                if (!body) return nullptr;
                auto parameters = function_type->get_parameters();
                initializer =
                    make_node<Zaban::AST::FunctionExpressionNode<COffsetType>>(
                        span(declarator->operations.front().range,
                             body->location()),
                        std::move(parameters), function_type->get_return_type(),
                        std::move(body));
                return make_node<Let>(
                    span(start_offset, this->previous()->range),
                    std::move(declarator->name), std::move(type),
                    std::move(initializer), std::move(specs.storage));
            }
            if (function_type) {
                auto parameters = function_type->get_parameters();
                initializer =
                    make_node<Zaban::AST::FunctionExpressionNode<COffsetType>>(
                        declarator->operations.front().range,
                        std::move(parameters), function_type->get_return_type(),
                        Zaban::AST::Statement<COffsetType>{});
            }
            if (this->check(CTokenKind::Equal)) {
                if (function_type) {
                    this->report(CParserDiagnosticKind::ErrorUnexpectedToken);
                    return nullptr;
                }
                this->advance();
                initializer = this->parse_initializer();
                if (!initializer) return nullptr;
                item_range = span(item_range, initializer->location());
            }
            declarations.push_back(
                // every declarator gets its own copy: `static int a, b;`
                make_node<Let>(item_range, std::move(declarator->name),
                               std::move(type), std::move(initializer),
                               specs.storage));
            if (!this->check(CTokenKind::Comma)) break;
            this->advance();
        }
        const auto* semi = this->expect(
            CTokenKind::Semicolon, CParserDiagnosticKind::ErrorExpectedToken);
        if (!semi) return nullptr;
        auto range = span(start_offset, semi->range);
        if (declarations.size() == 1) {
            declarations.front()->set_location(range);
            return declarations.front();
        }
        return make_node<AST::Declaration::DeclarationGroupNode<COffsetType>>(
            range, std::move(declarations));
    }

    // TODO: _Atomic is a qualifier unless followed by '(', where
    // `_Atomic(T)` is a type specifier. not handled yet.
    bool CParser::is_type_qualifier(CTokenKind kind) const {
        switch (kind) {
            case CTokenKind::Const:
            case CTokenKind::Volatile:
            case CTokenKind::Restrict:
                return true;
            default:
                return false;
        }
    }

    CParser::DeclarationSpecifiers CParser::parse_declaration_specifiers()
        const {
        return this->parse_specifiers(true);
    }

    CAnnotation CParser::parse_types() const {
        return this->parse_specifiers(false).type;
    }

    // Shared declaration specifiers; declarators own pointer operators.
    // Qualifiers, specifiers and storage classes may appear in any order
    // (`unsigned const long`, `int static x`), so they are collected in one
    // pass. qualifiers wrap the finished base type in a
    // QualifierAnnotationNode. storage classes are not part of the type and
    // are returned next to it. Pointer-level qualifiers (`int *const p`) are
    // parsed by the declarator instead. NOTE: specifiers are only collected
    // here. whether the combination is valid (C11 6.7.2p2), whether `restrict`
    // qualifies a pointer, and which storage classes may be combined, is
    // checked in semantic analysis.
    // allow_storage is false for the specifier-qualifier-list rule (members,
    // parameters, type names), where a storage class is an error.
    CParser::DeclarationSpecifiers CParser::parse_specifiers(
        bool allow_storage) const {
        using AST::Annotations::QualifierAnnotationNode;
        using AST::Annotations::TypeQualifier;
        using AST::Annotations::TypeSpecifier;
        using AST::Declaration::StorageClass;
        using Zaban::Langs::CLang::AST::Annotations::PrimitiveAnnotationNode;

        const auto* first = this->peek();
        if (!first) {
            this->report(CParserDiagnosticKind::ErrorUnexpectedEndOfFile);
            return {};
        }

        std::vector<TypeQualifier<COffsetType>> qualifiers{};
        std::vector<TypeSpecifier<COffsetType>> specifiers{};
        std::vector<StorageClass<COffsetType>>  storage{};
        // set only by struct/union/enum. it can't be mixed with keyword
        // specifiers.
        CAnnotation tagged = nullptr;
        // first and last token that belong to the type, so a storage class
        // before or after it stays out of the qualified type's range.
        std::optional<OffsetRange<COffsetType>> type_first;
        std::optional<OffsetRange<COffsetType>> type_last;
        const auto mark_type = [&](const auto* tok) {
            if (!type_first) type_first = tok->range;
            type_last = this->previous()->range;
        };

        while (const auto* tok = this->peek()) {
            if (is_type_qualifier(tok->kind)) {
                qualifiers.push_back(TypeQualifier{tok->kind, tok->range});
                this->advance();
                mark_type(tok);
                continue;
            }
            if (this->is_storage_class(tok->kind)) {
                if (!allow_storage) {
                    this->report(
                        CParserDiagnosticKind::ErrorDeclarationNotAllowed);
                    return {};
                }
                storage.push_back(StorageClass{tok->kind, tok->range});
                this->advance();
                continue;
            }
            if (tok->kind == CTokenKind::Struct ||
                tok->kind == CTokenKind::Union ||
                tok->kind == CTokenKind::Enum) {
                // `int struct S` or `struct A struct B`
                if (tagged || !specifiers.empty()) {
                    this->report(CParserDiagnosticKind::ErrorUnexpectedToken);
                    return {};
                }
                switch (tok->kind) {
                    case CTokenKind::Struct:
                        tagged = this->parse_struct_specifier();
                        break;
                    case CTokenKind::Union:
                        tagged = this->parse_union_specifier();
                        break;
                    default:
                        tagged = this->parse_enum_specifier();
                        break;
                }
                if (!tagged) return {};
                mark_type(tok);
                continue;
            }
            if (this->is_type_specifier(tok->kind)) {
                // `struct S int`
                if (tagged) {
                    this->report(CParserDiagnosticKind::ErrorUnexpectedToken);
                    return {};
                }
                specifiers.push_back(TypeSpecifier{tok->kind, tok->range});
                this->advance();
                mark_type(tok);
                continue;
            }
            break;
        }

        DeclarationSpecifiers result{};
        result.storage = std::move(storage);
        result.range   = span(first->range, this->previous()->range);

        CAnnotation base = std::move(tagged);
        if (!base) {
            // qualifiers or storage classes alone (`const x;`, `static x;`)
            // are not a type: no implicit int.
            if (specifiers.empty()) {
                this->report(CParserDiagnosticKind::ErrorExpectedType);
                return {};
            }
            auto range =
                span(specifiers.front().range, specifiers.back().range);
            base = make_node<PrimitiveAnnotationNode<COffsetType>>(
                range, std::move(specifiers));
        }

        if (qualifiers.empty()) {
            result.type = std::move(base);
            return result;
        }

        // covers every qualifier and specifier token, including qualifiers
        // that come before or after the base type.
        result.type = make_node<QualifierAnnotationNode<COffsetType>>(
            span(*type_first, *type_last), std::move(base),
            std::move(qualifiers));
        return result;
    }

    // The function suffix owns both parentheses. This parses only their
    // contents.
    std::optional<std::vector<CParameter>> CParser::parse_parameters() const {
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

        // nothing here can start a parameter, so the list has ended and the
        // missing ')' is the error. parse_function() reports it. a storage
        // class counts as a start so parse_types() can reject it by name.
        if (const auto* current = this->peek();
            !current || (!this->starts_declaration(current->kind) &&
                         current->kind != CTokenKind::Ellipsis)) {
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

            auto declarator =
                this->parse_declarator(DeclaratorNameMode::Optional);
            if (!declarator) return std::nullopt;
            if (!this->check_array_forms(*declarator, true)) {
                return std::nullopt;
            }
            auto param_type =
                this->build_declarator_type(std::move(type_), *declarator);
            auto range = span(start_offset, this->previous()->range);
            params.push_back(make_node<ParameterNode>(
                range, std::move(declarator->name), std::move(param_type)));
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

    // It does not consume an object name or declaration semicolon.
    CAnnotation CParser::parse_union_specifier() const {
        auto uni = this->expect(CTokenKind::Union,
                                CParserDiagnosticKind::ErrorExpectedToken);
        if (!uni) {
            return nullptr;
        }
        auto starting_range = uni->range;

        std::shared_ptr<Identifier> tag_name = nullptr;
        if (this->check(CTokenKind::Identifier)) {
            tag_name = make_node<Zaban::AST::Atomics::Identifier<COffsetType>>(
                this->advance()->range);
        }
        if (!this->check(TokenKind::LBrace)) {
            if (tag_name == nullptr) {
                this->report(CParserDiagnosticKind::ErrorExpectedIdentifier);
                return nullptr;
            }
            auto range = span(starting_range, tag_name->location());
            return make_node<UnionNode>(range, std::move(tag_name));
        }
        this->expect(CTokenKind::LBrace,
                     CParserDiagnosticKind::ErrorExpectedToken);

        auto members = parse_record_members();
        if (!members.has_value()) {
            return nullptr;
        }
        auto rbrace = this->expect(CTokenKind::RBrace,
                                   CParserDiagnosticKind::ErrorExpectedToken);
        if (!rbrace) {
            return nullptr;
        }
        return make_node<UnionNode>(span(starting_range, rbrace->range),
                                    std::move(tag_name),
                                    std::move(members.value()));
    }

    // It does not consume an object name or declaration semicolon.
    CAnnotation CParser::parse_struct_specifier() const {
        auto strct = this->expect(CTokenKind::Struct,
                                  CParserDiagnosticKind::ErrorExpectedToken);
        if (!strct) {
            return nullptr;
        }
        auto starting_range = strct->range;

        std::shared_ptr<Identifier> tag_name = nullptr;
        if (this->check(CTokenKind::Identifier)) {
            tag_name = make_node<Zaban::AST::Atomics::Identifier<COffsetType>>(
                this->advance()->range);
        }
        if (!this->check(TokenKind::LBrace)) {
            if (tag_name == nullptr) {
                this->report(CParserDiagnosticKind::ErrorExpectedIdentifier);
                return nullptr;
            }
            auto range = span(starting_range, tag_name->location());
            return make_node<StructNode>(range, std::move(tag_name));
        }
        this->expect(CTokenKind::LBrace,
                     CParserDiagnosticKind::ErrorExpectedToken);

        auto members = parse_record_members();
        if (!members.has_value()) {
            return nullptr;
        }
        auto rbrace = this->expect(CTokenKind::RBrace,
                                   CParserDiagnosticKind::ErrorExpectedToken);
        if (!rbrace) {
            return nullptr;
        }
        return make_node<StructNode>(span(starting_range, rbrace->range),
                                     std::move(tag_name),
                                     std::move(members.value()));
    }

    CAnnotation CParser::parse_enum_specifier() const {
        auto enm = this->expect(CTokenKind::Enum,
                                CParserDiagnosticKind::ErrorExpectedToken);
        if (!enm) {
            return nullptr;
        }
        auto starting_range = enm->range;

        std::shared_ptr<Identifier> tag_name = nullptr;
        if (this->check(CTokenKind::Identifier)) {
            tag_name = make_node<Zaban::AST::Atomics::Identifier<COffsetType>>(
                this->advance()->range);
        }
        if (!this->check(TokenKind::LBrace)) {
            if (tag_name == nullptr) {
                this->report(CParserDiagnosticKind::ErrorExpectedIdentifier);
                return nullptr;
            }
            auto range = span(starting_range, tag_name->location());
            return make_node<EnumNode>(range, std::move(tag_name));
        }
        this->expect(CTokenKind::LBrace,
                     CParserDiagnosticKind::ErrorExpectedToken);

        auto members = parse_enumerators();
        if (!members.has_value()) {
            return nullptr;
        }
        auto rbrace = this->expect(CTokenKind::RBrace,
                                   CParserDiagnosticKind::ErrorExpectedToken);
        if (!rbrace) {
            return nullptr;
        }
        return make_node<EnumNode>(span(starting_range, rbrace->range),
                                   std::move(tag_name),
                                   std::move(members.value()));
    }

    std::optional<std::vector<CParameter>> CParser::parse_enumerators() const {
        std::vector<CParameter> enums{};
        if (this->check(CTokenKind::RBrace)) {
            this->report(CParserDiagnosticKind::ErrorExpectedIdentifier);
            return enums;
        }
        while (!this->check(CTokenKind::RBrace) && !this->m_stream.end()) {
            if (this->check(CTokenKind::Semicolon)) break;
            const auto* start  = this->peek();
            auto        member = this->parse_enumerator();
            if (member) {
                enums.push_back(std::move(member));
            } else {
                this->synchronize_enumerator(start);
            }
            if (this->check(CTokenKind::RBrace) || this->m_stream.end()) break;
            if (this->check(CTokenKind::Semicolon)) break;
            if (this->check(CTokenKind::Comma)) {
                this->advance();
                continue;
            }
            this->report(CParserDiagnosticKind::ErrorExpectedToken);
            // A missing comma need not discard the following enumerator
            if (this->check(CTokenKind::Identifier)) continue;
            this->synchronize_enumerator(start);
            if (this->check(CTokenKind::Comma)) this->advance();
        }
        return enums;
    }

    CParameter CParser::parse_enumerator() const {
        auto name =
            this->expect(CTokenKind::Identifier,
                         CParserDiagnosticKind::ErrorExpectedIdentifier);
        if (!name) return nullptr;
        Zaban::AST::Atomic<COffsetType> identifier =
            make_node<Identifier>(name->range);
        CExpression value = nullptr;
        auto        range = name->range;
        if (this->check(CTokenKind::Equal)) {
            this->advance();
            value = this->parse_ternary();
            if (!value) return nullptr;
            range = span(range, value->location());
        }
        return make_node<ParameterNode>(range, std::move(identifier),
                                        std::move(value));
    }

    void CParser::synchronize_enumerator(const CTokenType* start) const {
        std::size_t braces   = 0;
        std::size_t parens   = 0;
        std::size_t brackets = 0;
        const auto  track    = [&](CTokenKind kind) {
            switch (kind) {
                case CTokenKind::LBrace:
                    ++braces;
                    break;
                case CTokenKind::RBrace:
                    if (braces) --braces;
                    break;
                case CTokenKind::LParen:
                    ++parens;
                    break;
                case CTokenKind::RParen:
                    if (parens) --parens;
                    break;
                case CTokenKind::LBrak:
                    ++brackets;
                    break;
                case CTokenKind::RBrak:
                    if (brackets) --brackets;
                    break;
                default:
                    break;
            }
        };
        for (auto token = start; token != this->peek(); ++token) {
            track(token->kind);
        }
        while (!this->m_stream.end()) {
            auto kind = this->peek()->kind;
            if (braces == 0) {
                // Even unmatched parentheses must not hide the enum's end.
                if (kind == CTokenKind::RBrace || kind == CTokenKind::Semicolon)
                    return;
                if (parens == 0 && brackets == 0 && kind == CTokenKind::Comma)
                    return;
            }
            track(kind);
            this->advance();
        }
    }

    std::optional<std::vector<CParameter>> CParser::parse_record_member()
        const {
        auto base = this->parse_types();
        if (!base) return std::nullopt;
        const auto              start = base->location();
        std::vector<CParameter> fields;
        do {
            if (!fields.empty()) this->advance();  // comma
            auto declarator = this->parse_declarator();
            if (!declarator) return std::nullopt;
            if (!this->check_array_forms(*declarator, false)) {
                return std::nullopt;
            }
            auto type = this->build_declarator_type(base, *declarator);
            // C members cannot have function type, but may point to functions.
            if (type->get_annotation_kind() ==
                Zaban::AST::AnnotationKind::Function) {
                this->report(CParserDiagnosticKind::ErrorDeclarationNotAllowed);
                return std::nullopt;
            }
            fields.push_back(make_node<ParameterNode>(
                declarator->range, std::move(declarator->name),
                std::move(type)));
        } while (this->check(CTokenKind::Comma));
        const auto* semi = this->expect(
            CTokenKind::Semicolon, CParserDiagnosticKind::ErrorExpectedToken);
        if (!semi) return std::nullopt;
        if (fields.size() == 1)
            fields.front()->set_location(span(start, semi->range));
        return fields;
    }

    std::optional<std::vector<CParameter>> CParser::parse_record_members()
        const {
        std::vector<CParameter> fields{};
        if (this->check(CTokenKind::RBrace)) {
            this->report(CParserDiagnosticKind::ErrorExpectedType);
            return fields;
        }
        while (!this->check(CTokenKind::RBrace) && !this->m_stream.end()) {
            const auto* start = this->peek();
            auto        field = this->parse_record_member();
            if (!field) {
                this->synchronize_record_member(start);
                continue;
            }
            for (auto& member: *field) fields.push_back(std::move(member));
        }
        return fields;
    }

    void CParser::synchronize_record_member(const CTokenType* start) const {
        std::size_t braces   = 0;
        std::size_t parens   = 0;
        std::size_t brackets = 0;
        const auto  track    = [&](CTokenKind kind) {
            switch (kind) {
                case CTokenKind::LBrace:
                    ++braces;
                    break;
                case CTokenKind::LParen:
                    ++parens;
                    break;
                case CTokenKind::LBrak:
                    ++brackets;
                    break;
                case CTokenKind::RBrace:
                    if (braces) --braces;
                    break;
                case CTokenKind::RParen:
                    if (parens) --parens;
                    break;
                case CTokenKind::RBrak:
                    if (brackets) --brackets;
                    break;
                default:
                    break;
            }
        };

        // Include delimiters consumed before the member parser failed. The
        // token stream keeps these pointers stable throughout parsing.
        for (auto token = start; token != this->peek(); ++token) {
            track(token->kind);
        }

        while (!this->m_stream.end()) {
            const auto kind = this->peek()->kind;
            // An unmatched bracket or paren must not hide the record's end.
            if (braces == 0 && kind == CTokenKind::RBrace) return;

            if (braces == 0 && parens == 0 && brackets == 0) {
                if (kind == CTokenKind::Semicolon) {
                    this->advance();
                    return;
                }
                // Preserve a following field when only a semicolon is missing.
                // If parsing consumed nothing, advance to guarantee progress.
                if (this->peek() != start && this->starts_declaration(kind)) {
                    return;
                }
            }
            track(kind);
            this->advance();
        }
    }
}  // namespace Z::Zaban::Langs::CLang
