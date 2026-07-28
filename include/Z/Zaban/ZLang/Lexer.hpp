#pragma once

#include <Z/Zaban/Lexer.hpp>
#include <Z/Zaban/ZLang/Token.hpp>
#include <string_view>

namespace Z::Zaban::ZLang {
    using ZLexerPositionType = std::size_t;
    using ZLexerFileRefType  = std::string_view;
    using ZLexerBufferType   = std::string_view;
    using ZLexerTokenKind    = Z::Zaban::ZLang::TokenKind;
    using ZLexerTokenType =
        Z::Zaban::ZLang::Token<ZLexerPositionType, ZLexerFileRefType>;

    class ZLexer : public Lexer<ZLexerTokenType, ZLexerPositionType,
                                ZLexerFileRefType, ZLexerBufferType> {
       private:
        ZLexerBufferType::const_reference peek(const ZLexerPositionType) const;
        ZLexerBufferType::const_reference peek(
            ZLexerBufferType::const_iterator) const;
        ZLexerBufferType::const_reference peek(ZLexerBufferType::const_iterator,
                                               ZLexerPositionType) const;

        ZLexerPositionType conditional_line_update(const char);

        void advance_offset();
        void advance_offset(const ZLexerPositionType);

       public:
        explicit ZLexer(ZLexerBufferType& buffer, ZLexerFileRefType file);

        void swap_buffer(ZLexerBufferType& buffer) override;

        ZLexerFileRefType  get_current_file() override;
        ZLexerPositionType get_current_line() override;
        ZLexerPositionType get_current_offset() override;

        void set_current_file(ZLexerFileRefType) override;
        void set_current_line(ZLexerPositionType) override;
        void set_current_offset(ZLexerPositionType) override;

        ZLexerTokenType                  get_token() override;
        ZLexerBufferType::const_iterator skip_trivial(
            ZLexerBufferType::const_iterator);
    };
}  // namespace Z::Zaban::ZLang
