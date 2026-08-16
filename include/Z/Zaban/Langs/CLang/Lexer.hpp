#pragma once

#include <Z/Zaban/Langs/CLang/Token.hpp>
#include <Z/Zaban/Langs/CLang/TokenKind.hpp>
#include <Z/Zaban/Lex/Lexer.hpp>
#include <Z/Zaban/Lex/LexerDiagnostics.hpp>
#include <cstdint>
#include <string_view>
#include <type_traits>
#include <vector>

namespace Z::Zaban::Langs::CLang {
    using CLexerPositionType = std::size_t;
    using CLexerBufferType   = std::string_view;
    using CLexerTokenKind    = Z::Zaban::Langs::CLang::TokenKind;
    using CLexerTokenType  = Z::Zaban::Langs::CLang::Token<CLexerPositionType>;
    using LexerDiagnostics = Z::Zaban::Lex::LexerDiagnostics;

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

    /// Controls which passes scan()/finalize() run.
    /// merge step can be suppressed while chunks are still being concatenated.
    enum class CLexerInvalidationFlag : std::uint8_t {
        None          = 0,
        NoScan        = 1 << 0,
        NoMergeTokens = 1 << 1,
    };

    constexpr CLexerInvalidationFlag& operator|=(CLexerInvalidationFlag& lhs,
                                                 CLexerInvalidationFlag  rhs) {
        using T = std::underlying_type_t<CLexerInvalidationFlag>;
        lhs     = static_cast<CLexerInvalidationFlag>(static_cast<T>(lhs) |
                                                      static_cast<T>(rhs));
        return lhs;
    }

    constexpr CLexerInvalidationFlag operator&(CLexerInvalidationFlag lhs,
                                               CLexerInvalidationFlag rhs) {
        using T = std::underlying_type_t<CLexerInvalidationFlag>;
        return static_cast<CLexerInvalidationFlag>(static_cast<T>(lhs) &
                                                   static_cast<T>(rhs));
    }

    constexpr CLexerInvalidationFlag& operator&=(CLexerInvalidationFlag& lhs,
                                                 CLexerInvalidationFlag  rhs) {
        using T = std::underlying_type_t<CLexerInvalidationFlag>;
        lhs     = static_cast<CLexerInvalidationFlag>(static_cast<T>(lhs) &
                                                      ~static_cast<T>(rhs));
        return lhs;
    }

    class CLexerDiagnostics : public LexerDiagnostics {
       public:
        bool has_errors() const override {
            return _error != CLexerError::None;
        }
        Lex::LexerErrorKind get_error_flags() const override {
            return Lex::LexerErrorKind::None;
        }
        std::size_t get_scan_count() const override {
            return _scan_count;
        }
        std::size_t get_concat_count() const override {
            return _concat_count;
        }
        void print_diagnostic_info(std::ostream&) const override {
        }

       private:
        CLexerError _error        = CLexerError::None;
        std::size_t _scan_count   = 0;
        std::size_t _concat_count = 0;
    };

    /** @brief Chunk-parallel lexical analyzer for C source.
     *
     * Each CLexer instance scans one buffer independently, starting at an
     * absolute base offset supplied at construction. scan() produces a token
     * stream terminated by an Eob marker. Two lexers that cover adjacent
     * chunks are combined with `<<` (concat), which splices their token
     * streams and re-runs the boundary merge so tokens split across the seam
     * are fused.
     *
     * This is the merge model: no per-token
     * state is carried between chunks. Correctness at the seam is recovered
     * afterwards by merge_double_tokens(), which fuses offset-contiguous
     * fragments per C maximal-munch rules.
     */
    class CLexer : public Zaban::Lex::Lexer<CLexerTokenType, CLexerPositionType,
                                            CLexerBufferType> {
        enum class CLexerInternalState {
            Normal,
            LineComment,
            BlockComment,
            String,
            CharLiteral,
        };

       private:
        CLexerError                      _error       = CLexerError::None;
        CLexerDiagnostics                _diagnostics = CLexerDiagnostics();
        CLexerInternalState              _state = CLexerInternalState::Normal;
        CLexerBufferType::const_iterator _buffer_it;
        std::vector<CLexerTokenType> _tokens = std::vector<CLexerTokenType>();
        CLexerInvalidationFlag       _flags  = CLexerInvalidationFlag::None;

        /// Absolute offset where the token under construction began.
        CLexerPositionType _token_start = 0;

        CLexerBufferType::const_pointer peek() const;
        CLexerBufferType::const_pointer peek(const CLexerPositionType) const;

        /// WARNING: nothing should touch _buffer_it outside advance()
        bool advance();
        bool advance(CLexerPositionType);
        bool eof() const;
        bool match_char(char);

        void invalidate(const CLexerInvalidationFlag);
        bool has_flag(const CLexerInvalidationFlag) const;
        void validate(const CLexerInvalidationFlag);

        void skip_line_comment_body(CLexerPositionType start);
        /// Finds the newline that closes an open line-comment fragment inside
        /// rhs. Returns the absolute offset of the newline, or -1 if rhs has
        /// none.
        CLexerPositionType scan_line_end_in_rhs(const CLexer& rhs) const;
        void               skip_block_comment_body(CLexerPositionType start);
        /// Finds the `*/` that closes an open comment fragment inside rhs.
        /// `frag_begin` is the absolute offset of the fragment's `/*`, used to
        /// reject `/*/` where the `*` is still part of the opener.
        /// Returns the absolute offset one past `*/`, or -1 if rhs doesn't
        /// close it.
        CLexerPositionType scan_comment_end_in_rhs(
            const CLexer& rhs, CLexerPositionType frag_begin) const;
        void skip_trivia();

        void lex_ident_keyword();
        void lex_number();
        void lex_string();
        void lex_char();
        void lex_punctuator();

        void push_token(CLexerTokenKind token);
        void push_token(CLexerTokenKind token, CLexerPositionType start,
                        CLexerPositionType end);

        /// Fuses offset-contiguous token fragments per C maximal-munch rules.
        /// Called after scan() and after each concat().
        void merge_double_tokens();

        /// Attempts to fuse the token at index i with the one at i+1.
        /// Returns the fused kind, or Dummy if they must not merge.
        CLexerTokenKind try_merge(const CLexerTokenType& a,
                                  const CLexerTokenType& b) const;

        void concat(const CLexer&);
        void concat(CLexer&&);

        CLexerPositionType scan_in_rhs(const CLexer& rhs, char delim,
                                       bool dangling) const;
        /// Closes this's trailing open literal fragment against rhs. On true,
        /// `out_tail` holds the re-lexed tokens for the part of rhs after the
        /// closing delimiter. Returns false if there was no open fragment.
        bool repair(const CLexer& rhs, std::vector<CLexerTokenType>& out_tail);

        void set_error(CLexerError err) {
            _error = err;
        }
        bool is_exponent_prefix(const char p) const {
            return (p == 'e' || p == 'E' || p == 'p' || p == 'P');
        }

       public:
        explicit CLexer(CLexerBufferType&);
        explicit CLexer(CLexerBufferType&, CLexerPositionType);

        CLexer& operator<<(const CLexer& rhs) {
            concat(rhs);
            return *this;
        }
        CLexer& operator<<(CLexer&& rhs) {
            concat(std::move(rhs));
            return *this;
        }

        void set_buffer(CLexerBufferType&) override;

        CLexerPositionType get_offset() override;
        void               set_offset(CLexerPositionType) override;

        bool                         scan() override;
        std::vector<CLexerTokenType> finalize() override;
        LexerDiagnostics&            diagnostics() override;
    };
}  // namespace Z::Zaban::Langs::CLang
