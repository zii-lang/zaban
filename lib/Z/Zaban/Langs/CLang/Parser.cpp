#include <Z/Zaban/Langs/CLang/Parser.hpp>

#include "Z/Zaban/Langs/CLang/ParserDiagnostic.hpp"
#include "Z/Zaban/SourcePosition.hpp"

namespace Z::Zaban::Langs::CLang {

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

    void CParser::synchronize() const {
        this->m_stream.advance();
        while (!this->m_stream.end()) {
            // a finished statement is a safe place to resume.
            const auto* prev = this->m_stream.previous();
            if (prev && prev->kind == CTokenKind::Semicolon) {
                return;
            }

            // otherwise stop at anything that starts a new statement or
            // declaration, or at '}' so the enclosing block can still close.
            const auto kind = this->m_stream.peek()->kind;
            if (this->is_type_specifier(kind)) {
                return;
            }
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
            this->m_stream.advance();
        }
    }

    CModule CParser::parse() {
        CModule module{};
        while (!m_stream.end()) {
            auto decl = parse_globals();
            if (decl == nullptr) {
                synchronize();
                continue;
            }
            module.push_back(decl);
        }

        return module;
    }

}  // namespace Z::Zaban::Langs::CLang
