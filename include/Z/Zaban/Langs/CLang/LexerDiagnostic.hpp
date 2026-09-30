#pragma once

#include <Z/Zaban/Lex/LexerDiagnostic.hpp>
#include <cstdint>
#include <string_view>

namespace Z::Zaban::Langs::CLang {
    enum class CLexerDiagnosticKind : std::uint32_t {
        InfoStart,
        InfoEnd,

        WarningStart,
        WarningInvalidEscapeSequence,
        WarningEnd,

        DeprecationStart,
        DeprecationEnd,

        ErrorStart,
        ErrorUnterminatedString,
        ErrorUnterminatedCharLiteral,
        ErrorUnterminatedComment,
        ErrorInvalidCharacter,
        ErrorUnexpectedEndOfFile,
        ErrorEnd,
    };

    class CLexerDiagnostic : public Lex::LexerDiagnostic<> {
       private:
        CLexerDiagnosticKind _kind;
        std::string_view     _reason;

       public:
        CLexerDiagnostic(CLexerDiagnosticKind kind, OffsetRange<> range) :
            CLexerDiagnostic(kind, "", range) {};

        CLexerDiagnostic(CLexerDiagnosticKind kind, std::string_view reason,
                         OffsetRange<> range) :
            Lex::LexerDiagnostic<>(range, Lex::LexerDiagnosticSeverity::Error),
            _kind(kind), _reason(reason) {
            if (CLexerDiagnosticKind::InfoStart < _kind &&
                _kind < CLexerDiagnosticKind::InfoEnd) {
                this->_severity = Lex::LexerDiagnosticSeverity::Info;
            } else if (CLexerDiagnosticKind::WarningStart < _kind &&
                       _kind < CLexerDiagnosticKind::WarningEnd) {
                this->_severity = Lex::LexerDiagnosticSeverity::Warning;
            } else if (CLexerDiagnosticKind::DeprecationStart < _kind &&
                       _kind < CLexerDiagnosticKind::DeprecationEnd) {
                this->_severity = Lex::LexerDiagnosticSeverity::Deprecation;
            }
        };

        CLexerDiagnosticKind kind() const {
            return this->_kind;
        }

        std::string_view reason() const {
            return this->_reason;
        }
    };

    class CLexerDiagnosticContext
        : public Lex::LexerDiagnosticContext<CLexerDiagnostic> {
       public:
        std::vector<CLexerDiagnostic> all() const {
            return this->_diag_vector;
        }
    };
}  // namespace Z::Zaban::Langs::CLang
