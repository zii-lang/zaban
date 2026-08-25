#pragma once

#include <Z/Zaban/Lex/LexerDiagnostic.hpp>
#include <string_view>

namespace Z::Zaban::Langs::ZLang {
    enum class ZLexerDiagnosticKind : std::uint32_t {
        InfoStart,
        InfoUnknown,
        InfoEnd,

        WarningStart,
        WarningUnknown,
        WarningEnd,

        DeprecationStart,
        DeprecationUnknown,
        DeprecationEnd,

        ErrorStart,
        ErrorUnknown,
        ErrorUnterminatedString,
        ErrorUnterminatedComment,
        ErrorInvalidCharacter,
        ErrorInvalidEscapeSequence,
        ErrorUnexpectedEndOfFile,
        ErrorEnd,
    };

    class ZLexerDiagnostic : public Lex::LexerDiagnostic<> {
       private:
        ZLexerDiagnosticKind _kind;
        std::string_view     _reason;

       public:
        ZLexerDiagnostic(ZLexerDiagnosticKind kind, OffsetRange<> range) :
            ZLexerDiagnostic(kind, "", range) {};

        ZLexerDiagnostic(ZLexerDiagnosticKind kind, std::string_view reason,
                         OffsetRange<> range) :
            Lex::LexerDiagnostic<>(range, Lex::LexerDiagnosticSeverity::Error),
            _kind(kind), _reason(reason) {
            if (ZLexerDiagnosticKind::InfoStart < _kind &&
                _kind < ZLexerDiagnosticKind::InfoEnd) {
                this->_severity = Lex::LexerDiagnosticSeverity::Info;
            } else if (ZLexerDiagnosticKind::WarningStart < _kind &&
                       _kind < ZLexerDiagnosticKind::WarningEnd) {
                this->_severity = Lex::LexerDiagnosticSeverity::Warning;
            } else if (ZLexerDiagnosticKind::DeprecationStart < _kind &&
                       _kind < ZLexerDiagnosticKind::DeprecationEnd) {
                this->_severity = Lex::LexerDiagnosticSeverity::Deprecation;
            } else if (ZLexerDiagnosticKind::ErrorStart < _kind &&
                       _kind < ZLexerDiagnosticKind::ErrorEnd) {
                this->_severity = Lex::LexerDiagnosticSeverity::Error;
            }
        };

        ZLexerDiagnostic(ZLexerDiagnosticKind kind, std::string_view reason,
                         OffsetRange<>                range,
                         Lex::LexerDiagnosticSeverity severity) :
            Lex::LexerDiagnostic<>(range, severity), _kind(kind),
            _reason(reason) {};

        ZLexerDiagnosticKind kind() {
            return this->_kind;
        }

        std::string_view reason() {
            return this->_reason;
        }
    };

    class ZLexerDiagnosticContext
        : public Lex::LexerDiagnosticContext<ZLexerDiagnostic> {
       public:
        std::vector<ZLexerDiagnostic> all() {
            return this->_diag_vector;
        }
    };
}  // namespace Z::Zaban::Langs::ZLang
