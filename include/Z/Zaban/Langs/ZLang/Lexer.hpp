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

       public:
        explicit ZLexer(ZLexerBufferType&);
        void set_buffer(ZLexerBufferType&) override;

        bool                         scan() override;
        std::vector<ZLexerTokenType> finalize() override;
        LexerDiagnostics             diagnostics() override;

        ZLexerBufferType::const_pointer peek() const;
        ZLexerBufferType::const_pointer peek(const ZLexerPositionType) const;
    };
}  // namespace Z::Zaban::Langs::ZLang
