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
     *
     * Every token type is handled uniformly with respect to chunk boundaries:
     * a token that is cut off has its consumed bytes accumulated into _pending
     * and its resume path recorded in _state. The next analyze() call replays
     * that path via resume() before returning to normal lexing.
     */
    class CLexer : public Zaban::Lex::Lexer<CLexerTokenType, CLexerPositionType,
                                            CLexerBufferType> {
        /// Names the lexing path to re-enter when a token was cut off by a
        /// chunk boundary. Normal means no token is in progress.
        enum class CLexerInternalState {
            Normal,
            Ident,
            Number,
            String,
            LineComment,
            BlockComment,
            CharLiteral,
        };

       private:
        CLexerError         _error = CLexerError::None;
        CLexerInternalState _state = CLexerInternalState::Normal;

        CLexerBufferType::const_iterator _buffer_it;
        /// Absolute offset where the token under construction began
        CLexerPositionType           _token_start = 0;
        std::vector<CLexerTokenType> _tokens;
        /// Bytes of the token under construction that were consumed in
        /// previous chunks. Empty unless a token was cut by a boundary.
        std::string _pending;

        /// it will be used to set the _token_start absolute offset.
        /// every lexing method will start by:
        /// this->_token_start = get_offset();
        ///
        /// that will allow token_start to be updated per token
        CLexerPositionType get_offset() override;

        /// WARNING: nothing should touch _buffer_it outside advance()
        bool advance();
        bool advance(CLexerPositionType);

        bool                            eof() const;
        CLexerBufferType::const_pointer get();
        /// Consumes any run of backslash newline pairs at the cursor
        /// must be called wherever a token may be interrupted by a line
        /// splice which in C is anywhere inside any token
        void skip_line_continuations();
        void skip_line_comment_body();
        void skip_block_comment_body();
        void skip_trivia();

        void lex_ident_keyword();
        void lex_number();
        void lex_string();
        void lex_char();
        void lex_punctuator();

        void lex_ident_body();
        void lex_number_body();
        void lex_string_body();
        void lex_char_body();

        /// re enters the path named by _state before normal lexing resumes
        void resume();
        /// Marks the current token as cut off: accumulates the bytes consumed
        /// so far in this chunk into _pending and records the resume path.
        void suspend(CLexerInternalState resume_state);

        void             push_token(CLexerTokenKind token);
        bool             match_char(char);
        CLexerBufferType current_lexeme();
        // TODO:
        void set_error(CLexerError err) {
            _error = err;
        }
        bool is_exponent_prefix(const char p) const {
            return (p == 'e' || p == 'E' || p == 'p' || p == 'P');
        }

       public:
        explicit CLexer(CLexerBufferType&);
        /// Handles replacing the previous buffer with a new one.
        /// * if any byte is unaccounted for, offset will keep them in check
        /// * checks if the incoming chunk starts where the last one ended
        /// * saves the incoming chunk's boundary
        /// * replaces the buffer with the new one
        void set_buffer(CLexerBufferType&) override;

        bool                         analyze() override;
        std::vector<CLexerTokenType> finalize() override;
        LexerDiagnostics             diagnostics() override;

        CLexerBufferType::const_pointer peek() const;
        CLexerBufferType::const_pointer peek(const CLexerPositionType) const;
    };
}  // namespace Z::Zaban::Langs::CLang
