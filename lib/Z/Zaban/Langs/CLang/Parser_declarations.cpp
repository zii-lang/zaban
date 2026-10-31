#include <Z/Zaban/AST/Annotation.hpp>
#include <Z/Zaban/AST/Annotations/ArrayAnnotation.hpp>
#include <Z/Zaban/AST/Annotations/PointerAnnotation.hpp>
#include <Z/Zaban/AST/Annotations/PrimitiveAnnotation.hpp>
#include <Z/Zaban/AST/Atomics/Identifier.hpp>
#include <Z/Zaban/AST/Declarations/LetDeclaration.hpp>
#include <Z/Zaban/AST/Expressions/FunctionExpression.hpp>
#include <Z/Zaban/AST/Parameter.hpp>
#include <Z/Zaban/Langs/CLang/AST/Annotations/EnumAnnotation.hpp>
#include <Z/Zaban/Langs/CLang/AST/Annotations/FunctionAnnotation.hpp>
#include <Z/Zaban/Langs/CLang/AST/Annotations/PrimitiveAnnotation.hpp>
#include <Z/Zaban/Langs/CLang/AST/Annotations/StructAnnotation.hpp>
#include <Z/Zaban/Langs/CLang/AST/Annotations/UnionAnnotation.hpp>
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

    std::optional<CParser::ParsedDeclarator> CParser::parse_declarator(
        DeclaratorNameMode mode) const {
        const auto* start = this->peek();
        if (!start || this->m_stream.end()) {
            this->report(CParserDiagnosticKind::ErrorUnexpectedEndOfFile);
            return std::nullopt;
        }

        std::vector<DeclaratorOp> pointers;

        while (this->check(CTokenKind::Asterisk)) {
            const auto* star = this->advance();

            pointers.push_back({
                DeclaratorOpKind::Pointer,
                star->range,
                nullptr,
                {},
            });
        }

        ParsedDeclarator result{
            nullptr,
            {},
            start->range,
        };

        if (mode != DeclaratorNameMode::Forbidden &&
            this->check(CTokenKind::Identifier)) {
            const auto* identifier = this->advance();

            result.name = make_node<Identifier>(identifier->range);
        } else if (this->starts_grouped_declarator(mode)) {
            this->advance();
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

            result = std::move(*nested);
        } else if (mode == DeclaratorNameMode::Required) {
            this->report(CParserDiagnosticKind::ErrorExpectedIdentifier);
            return std::nullopt;
        }

        while (this->check(CTokenKind::LBrak) ||
               this->check(CTokenKind::LParen)) {
            if (this->check(CTokenKind::LParen)) {
                const auto* opening    = this->advance();
                auto        parameters = this->parse_parameters();
                if (!parameters) return std::nullopt;
                const auto* closing =
                    this->expect(CTokenKind::RParen,
                                 CParserDiagnosticKind::ErrorExpectedToken);
                if (!closing) return std::nullopt;
                result.operations.push_back({
                    DeclaratorOpKind::Function,
                    span(opening->range, closing->range),
                    nullptr,
                    std::move(*parameters),
                });
                continue;
            }
            const auto* opening = this->advance();

            CExpression bound = nullptr;

            if (!this->check(CTokenKind::RBrak)) {
                bound = this->parse_assignment();
                if (!bound) return std::nullopt;
            }

            const auto* closing = this->expect(
                CTokenKind::RBrak, CParserDiagnosticKind::ErrorExpectedToken);

            if (!closing) return std::nullopt;

            result.operations.push_back({
                DeclaratorOpKind::Array,
                span(opening->range, closing->range),
                std::move(bound),
                {},
            });
        }

        // Prefix stars are farther from the identifier than this lvl's
        // suffixes. we should reverse their source order when appending them.
        for (auto it = pointers.rbegin(); it != pointers.rend(); ++it) {
            result.operations.push_back(std::move(*it));
        }

        if (this->peek() != start) {
            result.range = span(start->range, this->previous()->range);
        }
        return result;
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
                    base = make_node<ArrayAnnotation<COffsetType>>(
                        it->range, std::move(base), it->bound);
                    break;
                }
            }
        }
        return base;
    }

    // TODO: storage classes (static, extern) have no place in
    // LetDeclaration. add a C declaration node when they are parsed.
    CDeclaration CParser::parse_declaration(bool allow_body) const {
        // type
        auto base = this->parse_types();
        if (!base) return nullptr;
        auto start_offset = base->location();

        // Standalone type definitions or tag declarations. Enum definitions
        // may be unnamed because their enumerators introduce names.
        if (this->check(CTokenKind::Semicolon)) {
            auto struct_type = base->dyn_cast<StructNode>();
            if (struct_type && struct_type->get_tag()) {
                auto semi  = this->advance();
                auto range = span(start_offset, semi->range);
                return make_node<StructDeclaration>(range,
                                                    std::move(struct_type));
            }
            auto enum_type = base->dyn_cast<EnumNode>();
            if (enum_type &&
                (enum_type->get_tag() || enum_type->has_definition())) {
                auto semi = this->advance();
                return make_node<EnumDeclaration>(
                    span(start_offset, semi->range), std::move(enum_type));
            }
            auto union_type = base->dyn_cast<UnionNode>();
            if (union_type && union_type->get_tag()) {
                auto semi  = this->advance();
                auto range = span(start_offset, semi->range);
                return make_node<UnionDeclaration>(range,
                                                   std::move(union_type));
            }
        }

        using Let = Zaban::AST::Declarations::LetDeclaration<COffsetType>;
        using FunctionType =
            AST::Annotations::FunctionAnnotationNode<COffsetType>;
        std::vector<CDeclaration> declarations;
        while (true) {
            auto declarator = this->parse_declarator();
            if (!declarator) return nullptr;
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
                    std::move(initializer));
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
                make_node<Let>(item_range, std::move(declarator->name),
                               std::move(type), std::move(initializer)));
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

    // Shared declaration specifiers; declarators own pointer operators.
    // TODO: qualifiers (const, volatile) are not parsed. decide whether
    // they live on the annotation (they apply per pointer level: `int
    // *const p`). NOTE: specifiers are only collected here. whether the
    // combination is valid (C11 6.7.2p2) is checked in semantic
    // analysis.
    CAnnotation CParser::parse_types() const {
        using AST::Annotations::TypeSpecifier;
        using Zaban::AST::Annotations::PointerAnnotation;
        using Zaban::Langs::CLang::AST::Annotations::PrimitiveAnnotationNode;

        CAnnotation base        = nullptr;
        auto        current_tok = this->peek();
        if (!current_tok) return nullptr;
        switch (current_tok->kind) {
            case CTokenKind::Struct: {
                base = this->parse_struct_specifier();
                if (!base) return nullptr;
                break;
            }
            case CTokenKind::Enum: {
                base = this->parse_enum_specifier();
                if (!base) return nullptr;
                break;
            }
            case CTokenKind::Union: {
                base = this->parse_union_specifier();
                if (!base) return nullptr;
                break;
            }
            default: {
                std::vector<TypeSpecifier<COffsetType>> specifiers{};

                for (const auto* current = this->peek();
                     current && is_type_specifier(current->kind);) {
                    if (current->kind == CTokenKind::Struct ||
                        current->kind == CTokenKind::Union ||
                        current->kind == CTokenKind::Enum) {
                        this->report(CParserDiagnosticKind::ErrorExpectedType);
                        return nullptr;
                    }
                    specifiers.push_back(
                        TypeSpecifier{current->kind, current->range});

                    this->advance();
                    current = this->peek();
                }
                if (specifiers.empty()) {
                    report(CParserDiagnosticKind::ErrorExpectedType);
                    return nullptr;
                }
                auto range = span(specifiers[0].range, specifiers.back().range);
                base       = make_node<PrimitiveAnnotationNode<COffsetType>>(
                    range, std::move(specifiers));
            }
        }
        return base;
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
        // missing ')' is the error. parse_function() reports it.
        if (const auto* current = this->peek();
            !current || (!this->is_type_specifier(current->kind) &&
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
                if (this->peek() != start && this->is_type_specifier(kind)) {
                    return;
                }
            }
            track(kind);
            this->advance();
        }
    }
}  // namespace Z::Zaban::Langs::CLang
