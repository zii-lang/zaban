#pragma once

namespace Z::Zaban {
    enum class LexerErrorKind {
        UnterminatedString,
        UnterminatedComment,
        InvalidEscapeSequence,
        InvalidCharacter,
        UnexpectedEndOfFile,
    };

    class LexerDiagnostics {
       public:
        LexerDiagnostics() {};
    };
}  // namespace Z::Zaban
