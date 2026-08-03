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
        CLexerBufferType::const_pointer _prev_buffer_last = nullptr;

        CLexerBufferType::const_iterator _buffer_it;
        /// does this incoming chunk start exactly
        /// where the old one ended?
        /// if true, a token range may span the boundary
        bool _contiguous = false;
        /// Absolute offset where the token under construction began
        CLexerPositionType           _token_start = 0;
        std::vector<CLexerTokenType> _tokens;

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

        void skip_trivia();
        void lex_ident_keyword();
        void lex_number();
        void lex_string();
        void lex_char();
        void lex_punctuator();
        /// re enters the path named by _state before normal lexing resumes
        void             resume();
        void             push_token(CLexerTokenKind token);
        bool             match_char(char);
        CLexerBufferType current_lexeme() const;
        // TODO:
        void set_error(CLexerError err) {
            _error = err;
        }
        CLexerPositionType chunk_base() const {
            return _offset - (static_cast<CLexerPositionType>(_buffer_it -
                                                              _buffer.begin()));
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
