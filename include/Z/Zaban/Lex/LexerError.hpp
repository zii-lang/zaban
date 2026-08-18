#pragma once

namespace Z::Zaban::Lex {
    class LexerDiagnostics {
       public:
        LexerDiagnostics() {};
        virtual ~LexerDiagnostics() = default;

        virtual bool        has_errors() const       = 0;
        virtual std::size_t get_scan_count() const   = 0;
        virtual std::size_t get_concat_count() const = 0;
    };
}  // namespace Z::Zaban::Lex
