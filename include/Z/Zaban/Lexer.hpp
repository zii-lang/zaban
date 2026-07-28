#pragma once

#include <memory>

namespace Z::Zaban {
    template<typename T, typename P, typename F, typename B>
    class Lexer {
       private:
        using token_t    = T;
        using position_t = P;
        using file_ref_t = F;
        using buffer_t   = B;

       protected:
        buffer_t   current_buffer;
        file_ref_t current_file;
        position_t current_line;
        position_t current_offset;
        position_t offset_in_current_buffer;

       public:
        Lexer(buffer_t& buffer, file_ref_t file) :
            current_buffer(buffer), current_file(std::move(file)),
            current_line(1), current_offset(0), offset_in_current_buffer(0) {
        }
        virtual ~Lexer() = default;

        virtual void swap_buffer(buffer_t&) = 0;

        virtual file_ref_t get_current_file()   = 0;
        virtual position_t get_current_line()   = 0;
        virtual position_t get_current_offset() = 0;

        virtual void set_current_file(file_ref_t)   = 0;
        virtual void set_current_line(position_t)   = 0;
        virtual void set_current_offset(position_t) = 0;

        virtual token_t get_token() = 0;
    };
}  // namespace Z::Zaban
