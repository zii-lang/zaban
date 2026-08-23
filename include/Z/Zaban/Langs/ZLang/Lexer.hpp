#pragma once

#include <Z/Zaban/BitmaskEnum.hpp>
#include <Z/Zaban/Langs/ZLang/LexerDiagnostic.hpp>
#include <Z/Zaban/Langs/ZLang/ScanResult.hpp>
#include <Z/Zaban/Langs/ZLang/Token.hpp>
#include <Z/Zaban/Langs/ZLang/TokenKind.hpp>
#include <Z/Zaban/Lex/Lexer.hpp>
#include <Z/Zaban/Lex/LexerError.hpp>
#include <Z/Zaban/Lex/ScanUtil.hpp>
#include <string_view>
#include <unordered_map>

namespace Z::Zaban::Langs::ZLang {
    enum class ZLexerErrorFlag : std::uint8_t {
        None                  = 0,
        UnterminatedString    = 1 << 0,
        UnterminatedComment   = 1 << 1,
        InvalidEscapeSequence = 1 << 2,
        InvalidCharacter      = 1 << 3,
        UnexpectedEndOfFile   = 1 << 4,
    };

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
    using ZLexerTokenType    = ZLang::Token;
    using LexerDiagnostics   = Z::Zaban::Lex::LexerDiagnostics;

    enum class ZLexerSkipResult {
        /// Stopped because the next input is not trivial.
        NonTrivial,

        /// A trivial construct started but could not be completed.
        /// For example: an unterminated block comment.
        Incomplete,

        /// All remaining input was consumed successfully.
        EndOfInput,
    };

    static const std::unordered_map<std::string, ZLexerTokenKind>
        ZLangKeywords = {
            {"null", ZLexerTokenKind::Null},
            {"true", ZLexerTokenKind::True},
            {"false", ZLexerTokenKind::False},
            {"let", ZLexerTokenKind::Let},
            {"type", ZLexerTokenKind::Type},
            {"return", ZLexerTokenKind::Return},
            {"struct", ZLexerTokenKind::Struct},
            {"enum", ZLexerTokenKind::Enum},
            {"if", ZLexerTokenKind::If},
            {"endif", ZLexerTokenKind::EndIf},
            {"loop", ZLexerTokenKind::Loop},
            {"endloop", ZLexerTokenKind::EndLoop},
            {"func", ZLexerTokenKind::Func},
            {"vari", ZLexerTokenKind::Vari},
            {"break", ZLexerTokenKind::Break},
            {"continue", ZLexerTokenKind::Continue},
            {"goto", ZLexerTokenKind::Goto},
            {"label", ZLexerTokenKind::Label},
    };

    static bool is_identifier_start(const char ch) noexcept {
        return ch == '_' || Lex::CharUtil::is_alpha(ch);
    }

    static bool is_identifier_continue(const char ch) noexcept {
        return is_identifier_start(ch) || Lex::CharUtil::is_digit(ch);
    }

    static ZLexerTokenKind classify_identifier(const std::string& text) {
        const auto it = ZLangKeywords.find(text);

        if (it != ZLangKeywords.end()) {
            return it->second;
        }

        return ZLexerTokenKind::Identifier;
    }

    static std::string token_text(const ZLexer&          lexer,
                                  const ZLexerTokenType& token) {
        const auto buffer = lexer.get_buffer();

        const auto begin = static_cast<std::size_t>(token.range.begin -
                                                    lexer.get_start_offset());

        const auto end = static_cast<std::size_t>(token.range.end -
                                                  lexer.get_start_offset() + 1);

        if (begin > buffer.size() || end > buffer.size() || begin > end) {
            return {};
        }

        return std::string(buffer.data() + begin, end - begin);
    }

    class ZLexer : public Zaban::Lex::Lexer<ZLexerTokenType, ZLexerPositionType,
                                            ZLexerBufferType> {
       private:
        // ─────────────────────────────────────────────
        // Lexer state
        // ─────────────────────────────────────────────

        ZLexerInternalState _state = ZLexerInternalState::Normal;

        // ─────────────────────────────────────────────
        // Output
        // ─────────────────────────────────────────────

        std::vector<ZLexerTokenType> _tokens = std::vector<ZLexerTokenType>();

        // ─────────────────────────────────────────────
        // Diagnostics
        // ─────────────────────────────────────────────

        ZLexerDiagnosticContext _dc = ZLexerDiagnosticContext();

        // ─────────────────────────────────────────────
        // Pipeline state
        // ─────────────────────────────────────────────

        ZLexerInvalidationFlag _flags = ZLexerInvalidationFlag::NoScan |
                                        ZLexerInvalidationFlag::NoMergeTokens;

       private:
        // Pipeline

        ZLexerSkipResult skip_trivial();

        void concat(ZLexer&&);
        void concat(const ZLexer&);

       public:
        explicit ZLexer(ZLexerBufferType&);
        explicit ZLexer(ZLexerBufferType&, ZLexerPositionType);

        // Cursor

        [[nodiscard]]
        ZLexerBufferType::const_pointer peek() const noexcept;

        [[nodiscard]]
        ZLexerBufferType::const_pointer peek(
            const ZLexerPositionType distance) const noexcept;

        void advance();
        void advance(const ZLexerPositionType);

        bool                         scan() override;
        ScanResult                   scan_impl();
        std::vector<ZLexerTokenType> finalize() override;

        void merge();
        void merge(ZLexer& rhs);

        // Access
        [[nodiscard]] ZLexerInternalState get_state() const noexcept;
        void                              set_state(ZLexerInternalState state);

        [[nodiscard]] ZLexerBufferType get_buffer() const noexcept;
        void set_buffer(ZLexerBufferType& buffer) override;

        [[nodiscard]] ZLexerPositionType get_offset() override;
        void set_offset(ZLexerPositionType) override;

        [[nodiscard]] ZLexerPositionType get_start_offset() const noexcept;

        [[nodiscard]]
        std::vector<Token>& get_tokens();

        void set_tokens(std::vector<Token>);

        [[nodiscard]]
        Token& get_token(std::size_t);

        [[nodiscard]]
        Lex::LexerDiagnosticContextBase& diagnostic_ctx();

        [[nodiscard]]
        bool eob() const;

        ZLexer& operator<<(const ZLexer& rhs);
        ZLexer& operator<<(ZLexer&& rhs);
    };
}  // namespace Z::Zaban::Langs::ZLang
