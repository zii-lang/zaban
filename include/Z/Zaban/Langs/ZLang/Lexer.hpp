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

    class ZLexerDiagnostics : public LexerDiagnostics {};

    class ZLexer : public Zaban::Lex::Lexer<ZLexerTokenType, ZLexerPositionType,
                                            ZLexerBufferType> {
        enum class ZLexerInternalState {
            Normal,
            LineComment,
            BlockComment,
            String,
        };

       private:
        ZLexerError                      _error = ZLexerError::None;
        ZLexerInternalState              _state = ZLexerInternalState::Normal;
        ZLexerBufferType::const_pointer  _previous_buffer_last = nullptr;
        ZLexerBufferType::const_iterator _buffer_it;
        std::vector<ZLexerTokenType> _tokens = std::vector<ZLexerTokenType>();

        // Helper functions
        ZLexerBufferType::const_pointer peek() const;
        ZLexerBufferType::const_pointer peek(const ZLexerPositionType) const;

        void advance();
        void advance(const ZLexerPositionType);

        void set_lexer_state(const ZLexerInternalState);

        bool scan_newline();
        bool scan_until_newline();
        bool scan_double_slash_comment();
        bool scan_until_block_slash_comment();

        // Actual lexing
        void skip_trivial();

       public:
        explicit ZLexer(ZLexerBufferType&);
        void set_buffer(ZLexerBufferType&) override;

        bool                         analyze() override;
        std::vector<ZLexerTokenType> finalize() override;
        LexerDiagnostics             diagnostics() override;
    };
}  // namespace Z::Zaban::Langs::ZLang
