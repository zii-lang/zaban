#pragma once

#include <Z/Zaban/AST/Annotation.hpp>
#include <Z/Zaban/AST/Declaration.hpp>
#include <Z/Zaban/AST/Expression.hpp>
#include <Z/Zaban/AST/Module.hpp>
#include <Z/Zaban/AST/Parameter.hpp>
#include <Z/Zaban/AST/Statement.hpp>
#include <Z/Zaban/Langs/CLang/Lexer.hpp>
#include <Z/Zaban/Langs/CLang/ParserDiagnostic.hpp>
#include <Z/Zaban/Langs/CLang/TokenKind.hpp>
#include <Z/Zaban/Parse/Parser.hpp>
#include <Z/Zaban/Parse/TokenStream.hpp>
#include <concepts>
#include <cstddef>
#include <vector>

#include "Z/Zaban/SourcePosition.hpp"

namespace Z::Zaban::Langs::CLang {
    using COffsetType  = std::size_t;
    using CTokenKind   = CLang::TokenKind;
    using CTokenType   = Lex::Token<CTokenKind, COffsetType>;
    using CTokenStream = Parse::TokenStream<CTokenKind, COffsetType>;

    using CExpression  = Zaban::AST::Expression<COffsetType>;
    using CStatement   = Zaban::AST::Statement<COffsetType>;
    using CDeclaration = Zaban::AST::Declaration<COffsetType>;
    using CAnnotation  = Zaban::AST::Annotation<COffsetType>;
    using CParameter   = Zaban::AST::Parameter<COffsetType>;
    using CModule      = Zaban::AST::Module<COffsetType>;

    class CParser : public Parse::Parser<CTokenKind, COffsetType> {
       private:
        CTokenStream m_stream;

        mutable CParserDiagnosticContext m_diagnostics;

        // Nesting depth of loops, used to reject break/continue outside one.
        mutable std::size_t m_loop_depth = 0;

        // Top levels-> a function definition, forward declaration, or global.
        CDeclaration parse_globals() const;

        CDeclaration            parse_declaration() const;
        CAnnotation             parse_types() const;
        CAnnotation             parse_array_suffix(CAnnotation base) const;
        std::vector<CParameter> parse_parameters() const;
        CExpression             parse_initializer() const;

        CStatement parse_statement() const;
        CStatement parse_block_statement() const;
        CStatement parse_if_statement() const;
        CStatement parse_while_statement() const;
        CStatement parse_do_while_statement() const;
        CStatement parse_for_statement() const;
        CStatement parse_return_statement() const;
        CStatement parse_break_statement() const;
        CStatement parse_continue_statement() const;
        CStatement parse_expression_statement() const;

        // lowest to highest precedence
        CExpression parse_expression() const;
        CExpression parse_assignment() const;
        CExpression parse_ternary() const;
        CExpression parse_logical_or() const;
        CExpression parse_logical_and() const;
        CExpression parse_bitwise_or() const;
        CExpression parse_bitwise_xor() const;
        CExpression parse_bitwise_and() const;
        CExpression parse_equality() const;
        CExpression parse_comparison() const;
        CExpression parse_shifting() const;
        CExpression parse_additive() const;
        CExpression parse_multipicative() const;
        CExpression parse_unary() const;
        CExpression parse_suffix() const;
        CExpression parse_primary() const;
        CExpression parse_group() const;

        bool              is_type_specifier(CTokenKind kind) const;
        const CTokenType* expect(CTokenKind            kind,
                                 CParserDiagnosticKind diagnostic) const;
        void              report(CParserDiagnosticKind diagnostic) const;
        void              synchronize() const;
        bool              check(CTokenKind kind) const {
            return this->m_stream.check(kind);
        }
        const CTokenType* previous() const {
            return this->m_stream.previous();
        }
        const CTokenType* peek(std::size_t n = 0) const {
            return this->m_stream.peek(n);
        }
        const CTokenType* advance() const {
            return this->m_stream.advance();
        }

        template<typename T, typename... Args>
            requires std::derived_from<T, Zaban::AST::Node<COffsetType>>
        static std::shared_ptr<T> make_node(OffsetRange<COffsetType> range,
                                            Args&&... args) {
            auto node = std::make_shared<T>(std::forward<Args>(args)...);
            node->set_location(range);
            return node;
        }

        static inline OffsetRange<COffsetType> span(
            OffsetRange<COffsetType> first, OffsetRange<COffsetType> last) {
            return {first.begin, last.end};
        }

       public:
        CParser() = default;

        CModule parse(CTokenStream stream) override;

        const CParserDiagnosticContext& diagnostics() const {
            return this->m_diagnostics;
        }
    };
}  // namespace Z::Zaban::Langs::CLang
