#pragma once

#include <Z/Zaban/Lex/LexerDiagnostic.hpp>
#include <Z/Zaban/SourcePosition.hpp>
#include <algorithm>
#include <cstdint>
#include <string_view>
#include <vector>

namespace Z::Zaban::Langs::CLang {
    enum class CParserDiagnosticKind : std::uint32_t {
        InfoStart,
        InfoEnd,

        WarningStart,
        WarningEnd,

        DeprecationStart,
        DeprecationEnd,

        ErrorStart,
        ErrorUnexpectedEndOfFile,
        ErrorUnexpectedToken,
        ErrorExpectedToken,
        ErrorExpectedIdentifier,
        ErrorExpectedType,
        ErrorExpectedExpression,
        ErrorInvalidAssignmentTarget,
        ErrorArraySizeNotConstant,
        ErrorBreakOutsideLoop,
        ErrorContinueOutsideLoop,
        ErrorDeclarationNotAllowed,
        ErrorEnd,
    };

    // TODO: fix the inheritance later
    class CParserDiagnostic : public Lex::LexerDiagnostic<> {
       private:
        CParserDiagnosticKind _kind;
        std::string_view      _reason;

       public:
        CParserDiagnostic(CParserDiagnosticKind kind, OffsetRange<> range) :
            CParserDiagnostic(kind, "", range) {};

        CParserDiagnostic(CParserDiagnosticKind kind, std::string_view reason,
                          OffsetRange<> range) :
            Lex::LexerDiagnostic<>(range, Lex::LexerDiagnosticSeverity::Error),
            _kind(kind), _reason(reason) {
            if (_kind > CParserDiagnosticKind::InfoStart &&
                _kind < CParserDiagnosticKind::InfoEnd) {
                this->_severity = Lex::LexerDiagnosticSeverity::Info;
            } else if (_kind > CParserDiagnosticKind::WarningStart &&
                       _kind < CParserDiagnosticKind::WarningEnd) {
                this->_severity = Lex::LexerDiagnosticSeverity::Warning;
            } else if (_kind > CParserDiagnosticKind::DeprecationStart &&
                       _kind < CParserDiagnosticKind::DeprecationEnd) {
                this->_severity = Lex::LexerDiagnosticSeverity::Deprecation;
            }
        };

        CParserDiagnosticKind kind() const {
            return this->_kind;
        }

        std::string_view reason() const {
            return this->_reason;
        }
    };

    class CParserDiagnosticContext {
       private:
        std::vector<CParserDiagnostic> _diag_vector = {};

       public:
        void add(CParserDiagnostic diagnostic) {
            this->_diag_vector.push_back(std::move(diagnostic));
        }

        bool has_errors() const noexcept {
            return std::ranges::any_of(
                this->_diag_vector, [](const CParserDiagnostic& diagnostic) {
                    return diagnostic.severity() ==
                           Lex::LexerDiagnosticSeverity::Error;
                });
        }

        std::vector<CParserDiagnostic> all() const {
            return this->_diag_vector;
        }
    };
}  // namespace Z::Zaban::Langs::CLang
