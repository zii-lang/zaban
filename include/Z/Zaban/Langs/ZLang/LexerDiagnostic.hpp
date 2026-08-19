#pragma once

#include <Z/Zaban/Lex/LexerDiagnostic.hpp>

namespace Z::Zaban::Langs::ZLang {
    enum class ZLexerDiagnosticKind {
        InfoUnknown,
        WarningUnknown,
        DeprecationUnknown,
        ErrorUnknown,
        ErrorUnterminatedString,
        ErrorUnterminatedComment,
        ErrorInvalidCharacter,
        ErrorInvalidEscapeSequence,
        ErrorUnexpectedEndOfFile,
    };

    class ZLexerDiagnostic : public Lex::LexerDiagnostic<> {
       private:
        ZLexerDiagnosticKind _kind;
        std::string_view     _reason;

       public:
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
