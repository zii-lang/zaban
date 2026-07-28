#pragma once

#include <Z/Zaban/Lexer.hpp>
#include <Z/Zaban/ZLang/Token.hpp>
#include <string_view>

namespace Z::Zaban::ZLang {
    using ZLexerTokenKind    = Z::Zaban::ZLang::TokenKind;
    using ZLexerTokenType    = Z::Zaban::ZLang::Token;
    using ZLexerPositionType = std::size_t;
    using ZLexerFileRefType  = std::string;
    using ZLexerBufferType   = std::string_view;

    class ZLexer : public Lexer<ZLexerTokenType, ZLexerPositionType,
                                ZLexerFileRefType, ZLexerBufferType> {
       private:
       public:
        explicit ZLexer(ZLexerBufferType& buffer, ZLexerFileRefType file);

        void swap_buffer(ZLexerBufferType& buffer) override;

        ZLexerFileRefType  get_current_file() override;
        ZLexerPositionType get_current_line() override;
        ZLexerPositionType get_current_offset() override;

        void set_current_file(ZLexerFileRefType) override;
        void set_current_line(ZLexerPositionType) override;
        void set_current_offset(ZLexerPositionType) override;

        ZLexerTokenType get_token() override;
        void            skip_trivial() const;
    };
}  // namespace Z::Zaban::ZLang
