#pragma once

namespace Z::Zaban::Lex {
    enum class LexerErrorSeverity {
        Error,
        Deprecation,
        Warning,
        Info,
    };

    template<typename ErrorFlagType, typename PositionType>
    class LexerError {
       private:
        ErrorFlagType                     _error_kind;
        SourcePositionRange<PositionType> _source_range = {0, 0};
        LexerErrorSeverity                _severity = ZLexerErrorSeverity::Info;

       public:
        ZLexerError(ErrorFlagType kind, SourcePositionRange<PositionType> range,
                    LexerErrorSeverity severity) :
            _error_kind(kind), _source_range(range), _severity(severity) {
        }
    };

    template<typename ErrorFlagType, typename PositionType>
    class LexerDiagnostics {
       public:
        LexerDiagnostics() {};

        virtual bool has_errors() const = 0;
        virtual std::vector<LexerError<ErrorFlagType, PositionType>>
                            get_errors() const       = 0;
        virtual std::size_t get_scan_count() const   = 0;
        virtual std::size_t get_concat_count() const = 0;
    };
}  // namespace Z::Zaban::Lex
