#pragma once

namespace Z::Zaban {
    enum class LexerErrorKind {
        UnterminatedString,
        UnterminatedComment,
        InvalidEscapeSequence,
        InvalidCharacter,
        UnexpectedEndOfFile,
    };

    class LexerDiagnostics {};
}  // namespace Z::Zaban
