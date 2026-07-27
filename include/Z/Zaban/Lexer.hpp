#pragma once

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
        virtual Lexer(buffer_t&, file_ref_t) = default;
        virtual ~Lexer()                     = default;

        virtual void swap_buffer(buffer_t&) = default;

        virtual file_ref_t get_current_file()   = default;
        virtual position_t get_current_line()   = default;
        virtual position_t get_current_offset() = default;

        virtual void set_current_file(file_ref_t)   = default;
        virtual void set_current_line(position_t)   = default;
        virtual void set_current_offset(position_t) = default;

        virtual token_t get_token() = default;
    };
}  // namespace Z::Zaban
