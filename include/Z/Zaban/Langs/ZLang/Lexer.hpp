#pragma once

#include <Z/Zaban/Lexer.hpp>
#include <Z/Zaban/LexerDiagnostics.hpp>
#include <Z/Zaban/ZLang/Token.hpp>
#include <string_view>

namespace Z::Zaban::Langs::ZLang {
    using ZLexerPositionType = std::size_t;
    using ZLexerBufferType   = std::string_view;
    using ZLexerTokenKind    = Z::Zaban::ZLang::TokenKind;
    using ZLexerTokenType    = Z::Zaban::ZLang::Token<ZLexerPositionType>;

    enum class ZLexerError {
        None,
        UnterminatedString,
        UnterminatedComment,
        InvalidEscapeSequence,
        InvalidCharacter,
        UnexpectedEndOfFile,
    };

    class ZLexerDiagnostics : public LexerDiagnostics {};

    class ZLexer : public Lexer<ZLexerTokenType, ZLexerPositionType,
                                ZLexerFileRefType, ZLexerBufferType> {
        enum class ZLexerInternalState {
            Normal,
            LineComment,
            BlockComment,
            String,
        };

       private:
        ZLexerError         _error = ZLexerError::None;
        ZLexerInternalState _state = ZLexerInternalState::Normal;

        ZLexerBufferType::const_pointer _previous_buffer_last = nullptr;

        ZLexerBufferType::const_iterator _buffer_it;

        ZLexerBufferType::const_pointer peek() const;
        ZLexerBufferType::const_pointer peek(const ZLexerPositionType) const;
        ZLexerBufferType::const_pointer get();

        ZLexerPositionType get_current_line() override;
        ZLexerPositionType get_current_offset() override;

        ZLexerPositionType conditional_line_update(const char);

       public:
        explicit ZLexer(ZLexerBufferType& buffer, ZLexerFileRefType file);

        void swap_buffer(ZLexerBufferType& buffer) override;

        void set_current_file(ZLexerFileRefType) override;
        void set_current_line(ZLexerPositionType) override;
        void set_current_offset(ZLexerPositionType) override;

        ZLexerTokenType get_token() override;
        void            skip_trivial();

        bool advance();
        bool advance(ZLexerPositionType);

        ZLexerDiagnostics diagnostics();  // ?
    };
}  // namespace Z::Zaban::Langs::ZLang
