#pragma once

#include <Z/Zaban/BitmaskEnum.hpp>

namespace Z::Zaban::Lex {
    enum class LexerErrorKind : std::uint16_t {
        InvalidCharacter      = 1 << 0,
        UnterminatedString    = 1 << 1,
        UnterminatedComment   = 1 << 2,
        UnexpectedEndOfFile   = 1 << 3,
        InvalidEscapeSequence = 1 << 4,
    };

    class LexerDiagnostics {
       public:
        LexerDiagnostics() {};

        // virtual bool           has_errors()       = 0;
        // virtual LexerErrorKind get_error_flags()  = 0;
        virtual std::size_t get_scan_count() = 0;
        // virtual std::size_t    get_concat_count() = 0;
    };
}  // namespace Z::Zaban::Lex

namespace Z::Zaban {
    template<>
    struct enable_bitmask_operators<Z::Zaban::Lex::LexerErrorKind>
        : std::true_type {};
}  // namespace Z::Zaban
