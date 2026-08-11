#pragma once

#include <Z/Zaban/BitmaskEnum.hpp>
#include <Z/Zaban/Langs/ZLang/Token.hpp>
#include <Z/Zaban/Langs/ZLang/TokenKind.hpp>
#include <Z/Zaban/Lex/Lexer.hpp>
#include <Z/Zaban/Lex/LexerDiagnostics.hpp>
#include <string_view>

namespace Z::Zaban::Langs::ZLang {
    enum class ZLexerErrorFlag : std::uint8_t {
        None                  = 0,
        UnterminatedString    = 1 << 0,
        UnterminatedComment   = 1 << 1,
        InvalidEscapeSequence = 1 << 2,
        InvalidCharacter      = 1 << 3,
        UnexpectedEndOfFile   = 1 << 4,
    };

    enum class ZLexerInvalidationFlag : std::uint8_t {
        None          = 0,
        NoScan        = 1 << 0,
        NoMergeTokens = 1 << 1,
    };
}  // namespace Z::Zaban::Langs::ZLang

namespace Z::Zaban {
    Z_ENABLE_BITMASK_OPERATORS(Langs::ZLang::ZLexerErrorFlag);
    Z_ENABLE_BITMASK_OPERATORS(Langs::ZLang::ZLexerInvalidationFlag);
}  // namespace Z::Zaban

namespace Z::Zaban::Langs::ZLang {
    using ZLexerPositionType = std::size_t;
    using ZLexerBufferType   = std::string_view;
    using ZLexerTokenKind    = ZLang::TokenKind;
    using ZLexerTokenType    = ZLang::Token<ZLexerPositionType>;
    using LexerDiagnostics   = Z::Zaban::Lex::LexerDiagnostics;

    class ZLexerDiagnostics : public LexerDiagnostics {
        friend class ZLexer;

       private:
        ZLexerErrorFlag _errors       = ZLexerErrorFlag::None;
        std::size_t     _scan_count   = 0;
        std::size_t     _concat_count = 0;

        void increment_scan_count() {
            this->_scan_count++;
        }

        void increment_concat_count() {
            this->_concat_count++;
        }

       public:
        ZLexerDiagnostics() : LexerDiagnostics() {};

        bool has_errors() const override {
            return any(this->_errors);
        }

        Z::Zaban::Lex::LexerErrorKind get_error_flags() const override {
            using EKind = Z::Zaban::Lex::LexerErrorKind;
            EKind e     = EKind::None;

            if (!has_errors()) {
                return e;
            }
            if (has(this->_errors, ZLexerErrorFlag::UnterminatedComment)) {
                e = set(e, EKind::UnterminatedComment);
            }
            if (has(this->_errors, ZLexerErrorFlag::UnterminatedString)) {
                e = set(e, EKind::UnterminatedString);
            }
            if (has(this->_errors, ZLexerErrorFlag::InvalidCharacter)) {
                e = set(e, EKind::InvalidCharacter);
            }
            if (has(this->_errors, ZLexerErrorFlag::InvalidEscapeSequence)) {
                e = set(e, EKind::InvalidEscapeSequence);
            }
            if (has(this->_errors, ZLexerErrorFlag::UnexpectedEndOfFile)) {
                e = set(e, EKind::UnexpectedEndOfFile);
            }
            return e;
        }

        std::size_t get_scan_count() const override {
            return this->_scan_count;
        }

        std::size_t get_concat_count() const override {
            return this->_concat_count;
        }

        void print_diagnostic_info(std::ostream& out) const override {
            out << "Lexer diagnostic information\n";
            out << "============================\n";
            out << (has_errors() ? "Has Errors\n" : "No Errors\n");
            out << "Scanned total time of: " << get_scan_count() << '\n';
            out << "Concat with other lexers: " << get_concat_count()
                << " times.\n";
            out << "Error flags: "
                << static_cast<std::uint16_t>(get_error_flags()) << '\n';
        }
    };

    class ZLexer : public Zaban::Lex::Lexer<ZLexerTokenType, ZLexerPositionType,
                                            ZLexerBufferType> {
        enum class ZLexerInternalState {
            Error,
            Normal,
            LineComment,
            BlockComment,
            SQString,
            DQString,

            Identifier,

            STATE_NumStart,
            /// Number lexing started with 0
            ZeroStart,
            /// 0x
            HexNumber,
            /// 0o
            OctNumber,
            /// 0b
            BinNumber,
            // 0 [digit] or [digit] scanned and we are now in number mode.
            Number,
            /// [digit]* "." lexed so we don't become float again.
            FloatNumber,
            /// From float mode into scientific mode.
            ScientificNumber,
            STATE_NumEnd,
        };

       private:
        ZLexerDiagnostics                _diagnostics = ZLexerDiagnostics();
        ZLexerInternalState              _state = ZLexerInternalState::Normal;
        ZLexerBufferType::const_iterator _buffer_it;
        std::vector<ZLexerTokenType> _tokens = std::vector<ZLexerTokenType>();

        ZLexerInvalidationFlag _flags = ZLexerInvalidationFlag::NoScan |
                                        ZLexerInvalidationFlag::NoMergeTokens;

        // Helper functions
        ZLexerBufferType::const_pointer peek() const;
        ZLexerBufferType::const_pointer peek(const ZLexerPositionType) const;

        void invalidate(const ZLexerInvalidationFlag);
        bool has_flag(const ZLexerInvalidationFlag);
        void validate(const ZLexerInvalidationFlag);
        void validate_all();

        void advance();
        void advance(const ZLexerPositionType);

        void set_lexer_state(const ZLexerInternalState);

        bool scan_until(ZLexerBufferType::value_type);
        bool scan_newline();
        bool scan_until_newline();
        bool scan_comment();
        bool scan_double_slash_close_comment();
        bool scan_until_block_slash_close_comment();
        bool scan_until_eos();
        bool scan_until_get_numeric();

        // Actual lexing
        void skip_trivial();

        // Merge double tokens
        void merge_double_tokens();
        void merge_identifier_boundary(ZLexer& rhs);
        void concat(const ZLexer&);
        void concat(ZLexer&&);

       public:
        explicit ZLexer(ZLexerBufferType&);
        explicit ZLexer(ZLexerBufferType&, ZLexerPositionType);

        ZLexer& operator<<(const ZLexer& rhs) {
            concat(rhs);
            return *this;
        }

        ZLexer& operator<<(ZLexer&& rhs) {
            concat(std::move(rhs));
            return *this;
        }
        ZLexerBufferType get_buffer() const;
        void             set_buffer(ZLexerBufferType&) override;

        ZLexerPositionType get_offset() override;
        void               set_offset(ZLexerPositionType) override;

        ZLexerPositionType get_start_offset() const;

        bool                         scan() override;
        std::vector<ZLexerTokenType> finalize() override;
        LexerDiagnostics&            diagnostics() override;
    };
}  // namespace Z::Zaban::Langs::ZLang
