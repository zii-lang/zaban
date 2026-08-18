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

       public:
        ZLexerDiagnostic(ZLexerDiagnosticKind kind, OffsetRange<> range,
                         Lex::LexerDiagnosticSeverity severity) :
            Lex::LexerDiagnostic<>(range, severity), _kind(kind) {};

        ZLexerDiagnosticKind kind() {
            return this->_kind;
        }
    };

    class ZLexerDiagnosticContext
        : public Lex::LexerDiagnosticContext<ZLexerDiagnostic> {};
}  // namespace Z::Zaban::Langs::ZLang
