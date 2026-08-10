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
       private:
        std::size_t _scan_count = 0;

       public:
        ZLexerDiagnostics() : LexerDiagnostics() {};

        void increment_scan_count() {
            this->_scan_count++;
        }

        void set_scan_count(std::size_t count) {
            this->_scan_count = count;
        }

        std::size_t get_scan_count() override {
            return this->_scan_count;
        }
    };

    class ZLexer : public Zaban::Lex::Lexer<ZLexerTokenType, ZLexerPositionType,
                                            ZLexerBufferType> {
        enum class ZLexerInternalState {
            Normal,
            LineComment,
            BlockComment,
            SQString,
            DQString,
        };

       private:
        ZLexerDiagnostics                _diagnostics = ZLexerDiagnostics();
        ZLexerInternalState              _state = ZLexerInternalState::Normal;
        ZLexerBufferType::const_iterator _buffer_it;
        std::vector<ZLexerTokenType> _tokens = std::vector<ZLexerTokenType>();

        ZLexerErrorFlag        _error = ZLexerErrorFlag::None;
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

        // Actual lexing
        void skip_trivial();

        // Merge double tokens
        void merge_double_tokens();
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

        void set_buffer(ZLexerBufferType&) override;

        ZLexerPositionType get_offset() override;
        void               set_offset(ZLexerPositionType) override;

        bool                         scan() override;
        std::vector<ZLexerTokenType> finalize() override;
        LexerDiagnostics&            diagnostics() override;
    };
}  // namespace Z::Zaban::Langs::ZLang
