#include <Z/Zaban/Langs/CLang/Parser.hpp>
#include <cassert>

#include "Z/Zaban/Langs/CLang/ParserDiagnostic.hpp"
#include "Z/Zaban/SourcePosition.hpp"

namespace Z::Zaban::Langs::CLang {
    namespace {
        // the preprocessor keeps directive lines, skipped groups and the
        // lexer's Eob terminator. the parser needs none of them and stops at
        // Eof.
        std::vector<CTokenType> parser_tokens(std::vector<CTokenType> tokens) {
            // the main buffer's trailing Eob ends the translation unit.
            // interior ones (from included files) are dropped along with it.
            assert(!tokens.empty() && tokens.back().kind == CTokenKind::Eob);
            const OffsetRange<COffsetType> end =
                tokens.empty() ? OffsetRange<COffsetType>(0, 0)
                               : tokens.back().range;

            std::vector<CTokenType> out;
            out.reserve(tokens.size());
            for (auto& token: tokens) {
                const auto flags = static_cast<TokenFlags>(token.flags);
                if (has(flags, TokenFlags::DirectiveLine)) continue;
                if (has(flags, TokenFlags::Skipped)) continue;
                if (token.kind == CTokenKind::Eob) continue;
                out.push_back(std::move(token));
            }
            out.emplace_back(CTokenKind::Eof, end);
            return out;
        }
    }  // namespace

    CParser::CParser(std::vector<CTokenType> preprocessed) :
        m_stream(parser_tokens(std::move(preprocessed))) {
    }

    bool CParser::is_type_specifier(CTokenKind kind) const {
        // TODO: typedef names are plain identifiers and can't be recognized
        // here. that needs a typedef table. qualifiers (const, volatile) and
        // storage classes (static, extern) aren't type specifiers either.
        switch (kind) {
            case CTokenKind::Void:
            case CTokenKind::Char:
            case CTokenKind::Short:
            case CTokenKind::Int:
            case CTokenKind::Long:
            case CTokenKind::Float:
            case CTokenKind::Double:
            case CTokenKind::Signed:
            case CTokenKind::Unsigned:
            case CTokenKind::Bool:
            case CTokenKind::Struct:
            case CTokenKind::Union:
            case CTokenKind::Enum:
                return true;
            default:
                return false;
        }
    }

    const CTokenType* CParser::expect(CTokenKind            kind,
                                      CParserDiagnosticKind diagnostic) const {
        if (this->m_stream.check(kind)) return this->m_stream.advance();

        report(diagnostic);
        return nullptr;
    }

    void CParser::report(CParserDiagnosticKind diagnostic) const {
        assert(this->peek() != nullptr || this->previous() != nullptr);
        if (auto current = this->m_stream.peek()) {
            this->m_diagnostics.add(
                CParserDiagnostic{diagnostic, current->range});
        } else if (auto prev = this->m_stream.previous()) {
            this->m_diagnostics.add(CParserDiagnostic{diagnostic, prev->range});
        }
    }

    void CParser::report_at(CParserDiagnosticKind    diagnostic,
                            OffsetRange<COffsetType> range) const {
        this->m_diagnostics.add(CParserDiagnostic(diagnostic, range));
    }

    void CParser::reject_declaration(const CStatement& stmt) const {
        if (stmt->statement_kind() == Zaban::AST::StatementKind::Declaration) {
            report_at(CParserDiagnosticKind::ErrorDeclarationNotAllowed,
                      stmt->location());
        }
    }

    void CParser::synchronize(bool top_lvl) const {
        // at top level a broken declaration's body is skipped whole. nothing
        // inside it is a place to resume, its locals look like globals.
        std::size_t depth = 0;
        const auto  skip  = [&] {
            const auto* token = this->m_stream.advance();
            if (!top_lvl || !token) return;
            if (token->kind == CTokenKind::LBrace) {
                ++depth;
            } else if (token->kind == CTokenKind::RBrace && depth > 0) {
                --depth;
            }
        };

        skip();
        while (!this->m_stream.end()) {
            // a finished statement is a safe place to resume.
            const auto* prev = this->m_stream.previous();
            if (depth == 0 && prev && prev->kind == CTokenKind::Semicolon) {
                return;
            }

            // otherwise stop at anything that starts a new statement or
            // declaration, or at '}' so the enclosing block can still close.
            const auto kind = this->m_stream.peek()->kind;
            if (depth == 0 && this->is_type_specifier(kind)) {
                return;
            }
            if (!top_lvl) {
                switch (kind) {
                    case CTokenKind::If:
                    case CTokenKind::While:
                    case CTokenKind::Do:
                    case CTokenKind::For:
                    case CTokenKind::Switch:
                    case CTokenKind::Return:
                    case CTokenKind::Break:
                    case CTokenKind::Continue:
                    case CTokenKind::Goto:
                    case CTokenKind::RBrace:
                        return;
                    default:
                        break;
                }
            }
            skip();
        }
    }

    CModule CParser::parse() {
        CModule module{};
        while (!m_stream.end()) {
            auto decl = parse_globals();
            if (decl == nullptr) {
                synchronize(true);
                continue;
            }
            module.push_back(decl);
        }

        return module;
    }

}  // namespace Z::Zaban::Langs::CLang
