#pragma once

#include <Z/Zaban/Langs/ZLang/Token.hpp>
#include <Z/Zaban/Langs/ZLang/TokenKind.hpp>
#include <Z/Zaban/Lex/Lexer.hpp>
#include <Z/Zaban/Lex/LexerDiagnostics.hpp>
#include <string_view>

namespace Z::Zaban::Langs::ZLang {
    using ZLexerPositionType = std::size_t;
    using ZLexerBufferType   = std::string_view;
    using ZLexerTokenKind    = Z::Zaban::Langs::ZLang::TokenKind;
    using ZLexerTokenType = Z::Zaban::Langs::ZLang::Token<ZLexerPositionType>;

    enum class ZLexerError {
        None,
        UnterminatedString,
        UnterminatedComment,
        InvalidEscapeSequence,
        InvalidCharacter,
        UnexpectedEndOfFile,
    };

    enum class ZLexerInvalidationFlag : std::uint8_t {
        None          = 0,
        NoScan        = 1 << 0,
        NoMergeTokens = 1 << 1,
    };

    constexpr ZLexerInvalidationFlag& operator|=(ZLexerInvalidationFlag& lhs,
                                                 ZLexerInvalidationFlag  rhs) {
        using T = std::underlying_type_t<ZLexerInvalidationFlag>;

        lhs = static_cast<ZLexerInvalidationFlag>(static_cast<T>(lhs) |
                                                  static_cast<T>(rhs));

        return lhs;
    }

    constexpr ZLexerInvalidationFlag operator&(ZLexerInvalidationFlag& lhs,
                                               ZLexerInvalidationFlag  rhs) {
        using T = std::underlying_type_t<ZLexerInvalidationFlag>;
        ZLexerInvalidationFlag flag = static_cast<ZLexerInvalidationFlag>(
            static_cast<T>(lhs) | static_cast<T>(rhs));

        return flag;
    }

    constexpr ZLexerInvalidationFlag& operator&=(ZLexerInvalidationFlag& lhs,
                                                 ZLexerInvalidationFlag  rhs) {
        using U = std::underlying_type_t<ZLexerInvalidationFlag>;

        lhs = static_cast<ZLexerInvalidationFlag>(static_cast<U>(lhs) &
                                                  ~static_cast<U>(rhs));

        return lhs;
    }

    class ZLexerDiagnostics : public LexerDiagnostics {};

    class ZLexer : public Zaban::Lex::Lexer<ZLexerTokenType, ZLexerPositionType,
                                            ZLexerBufferType> {
        enum class ZLexerInternalState {
            Normal,
            Whitespace,
            LineComment,
            BlockComment,
            String,
        };

       private:
        ZLexerError                      _error = ZLexerError::None;
        ZLexerInternalState              _state = ZLexerInternalState::Normal;
        ZLexerBufferType::const_iterator _buffer_it;
        std::vector<ZLexerTokenType> _tokens = std::vector<ZLexerTokenType>();

        ZLexerInvalidationFlag _flags = ZLexerInvalidationFlag::None;

        // Helper functions
        ZLexerBufferType::const_pointer peek() const;
        ZLexerBufferType::const_pointer peek(const ZLexerPositionType) const;

        void invalidate(const ZLexerInvalidationFlag);
        bool has_flag(const ZLexerInvalidationFlag);
        void validate();

        void advance();
        void advance(const ZLexerPositionType);

        void set_lexer_state(const ZLexerInternalState);

        bool scan_newline();
        bool scan_until_newline();
        bool scan_comment();
        bool scan_double_slash_close_comment();
        bool scan_until_block_slash_close_comment();

        // Actual lexing
        void skip_trivial();

        // Merge double tokens
        void merge_double_tokens();
        void concat(const ZLexer&);
        void concat(ZLexer&&);

       public:
        explicit ZLexer(ZLexerBufferType&);

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
        LexerDiagnostics             diagnostics() override;
    };
}  // namespace Z::Zaban::Langs::ZLang
