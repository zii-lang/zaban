#pragma once

#include <Z/Zaban/BitmaskEnum.hpp>
#include <ostream>

namespace Z::Zaban::Lex {
    enum class LexerErrorKind : std::uint16_t {
        None                  = 0,
        InvalidCharacter      = 1 << 0,
        UnterminatedString    = 1 << 1,
        UnterminatedComment   = 1 << 2,
        UnexpectedEndOfFile   = 1 << 3,
        InvalidEscapeSequence = 1 << 4,
    };

    class LexerDiagnostics {
       public:
        LexerDiagnostics() {};
        virtual ~LexerDiagnostics() = default;

        virtual bool           has_errors() const                   = 0;
        virtual LexerErrorKind get_error_flags() const              = 0;
        virtual std::size_t    get_scan_count() const               = 0;
        virtual std::size_t    get_concat_count() const             = 0;
        virtual void print_diagnostic_info(std::ostream& out) const = 0;
    };
}  // namespace Z::Zaban::Lex

namespace Z::Zaban {
    template<>
    struct enable_bitmask_operators<Z::Zaban::Lex::LexerErrorKind>
        : std::true_type {};
}  // namespace Z::Zaban
