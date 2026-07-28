#pragma once

#include <concepts>
#include <memory>

namespace Z::Zaban::Lex {
    template<typename T, typename P, typename B>
        requires std::integral<P>
    class Lexer {
       private:
        using LexerTokenType    = T;
        using LexerBufferType   = B;
        using LexerPositionType = P;

        virtual LexerPositionType get_offset() = 0;

       protected:
        buffer_t   _buffer;
        position_t _line;
        position_t _offset;

       public:
        Lexer(buffer_t& buffer) : _buffer(buffer), _offset(0) {
        }
        virtual ~Lexer() = default;

        virtual void set_buffer(buffer_t&)  = 0;
        virtual void set_offset(position_t) = 0;
    };
}  // namespace Z::Zaban::Lex
