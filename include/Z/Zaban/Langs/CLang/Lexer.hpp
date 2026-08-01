#pragma once

#include <Z/Zaban/Langs/CLang/Token.hpp>
#include <Z/Zaban/Langs/CLang/TokenKind.hpp>
#include <Z/Zaban/Lex/Lexer.hpp>
#include <Z/Zaban/Lex/LexerDiagnostics.hpp>
#include <string_view>

namespace Z::Zaban::Langs::CLang {
    using CLexerPositionType = std::size_t;
    using CLexerBufferType   = std::string_view;
    using CLexerTokenKind    = Z::Zaban::Langs::CLang::TokenKind;
    using CLexerTokenType = Z::Zaban::Langs::CLang::Token<CLexerPositionType>;

    enum class CLexerError {
        None,
        UnterminatedString,
        UnterminatedCharLiteral,
        UnterminatedComment,
        InvalidEscapeSequence,
        InvalidNumericLiteral,
        InvalidCharacter,
        UnexpectedEndOfFile,
    };

    class CLexerDiagnostics : public LexerDiagnostics {};

    /** @brief Incremental lexical analyzer for C source.
     *
     * The lexer consumes the source in chunks. Each call to analyze() reports
     * whether the chunk formed a complete source or whether the trailing
     * bytes belong to a construct that continues into the next chunk.
     */
    class CLexer : public Zaban::Lex::Lexer<CLexerTokenType, CLexerPositionType,
                                            CLexerBufferType> {
        enum class CLexerInternalState {
            Normal,
            LineComment,
            BlockComment,
            String,
            CharLiteral,
            /// A token whose spelling was cut by the chunk boundary.
            /// _last_token holds the longest match found so far.
            MultiCharToken,
        };

       private:
        CLexerError         _error = CLexerError::None;
        CLexerInternalState _state = CLexerInternalState::Normal;

        /// Longest match produced so far for a token split across chunks
        /// only meaningful while _state is MultiCharTOken
        CLexerTokenKind _last_token = CLexerTokenKind::Dummy;
        /// One past the last byte of the prev chunk. used to detect
        /// whether the incoming chunk is contiguous with it in memory
        CLexerBufferType::const_pointer  _prev_buffer_last = nullptr;
        CLexerBufferType::const_iterator _buffer_it;
        /// Absolute offset of _buffer[0] within the module translation unit
        CLexerPositionType _chunk_base = 0;
        /// Absolute offset where the token under construction began
        CLexerPositionType           _token_start = 0;
        std::vector<CLexerTokenType> _tokens;
        CLexerPositionType           get_offset() override;

        bool                            advance();
        bool                            advance(CLexerPositionType);
        bool                            eof() const;
        CLexerBufferType::const_pointer get();
        /// Consumes any run of backslash newline pairs at the cursor
        /// must be called wherever a token may be interrupted by a line
        /// splice which in C is anywhere inside any token
        void skip_line_continuations();

        void skip_trivia();
        void lex_ident_keyword();
        void lex_number();
        void lex_string();
        void lex_char();
        void lex_punctuator();
        /// re enters the path named by _state before normal lexing resumes
        void resume();
        void push_token();
        void set_error();

       public:
        explicit CLexer(CLexerBufferType&);
        void set_buffer(CLexerBufferType&) override;

        bool                         analyze() override;
        std::vector<CLexerTokenType> finalize() override;
        LexerDiagnostics             diagnostics() override;

        CLexerBufferType::const_pointer peek() const;
        CLexerBufferType::const_pointer peek(const CLexerPositionType) const;
    };
}  // namespace Z::Zaban::Langs::CLang
