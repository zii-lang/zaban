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

        virtual std::size_t scan_count() = 0;
    };
}  // namespace Z::Zaban
