#pragma once

#include <Z/Zaban/Langs/CLang/LexerDiagnostic.hpp>
#include <Z/Zaban/Langs/CLang/LexerTypes.hpp>
#include <Z/Zaban/Langs/CLang/Token.hpp>
#include <Z/Zaban/Langs/CLang/TokenKind.hpp>
#include <Z/Zaban/Lex/Lexer.hpp>
#include <cstdint>
#include <string_view>
#include <vector>

#include "Z/Zaban/BitmaskEnum.hpp"

namespace Z::Zaban::Langs::CLang {
    using CLexerTokenKind = Z::Zaban::Langs::CLang::TokenKind;
    using CLexerTokenType = Z::Zaban::Langs::CLang::Token;

    /// Controls which passes scan()/finalize() run.
    /// merge step can be suppressed while chunks are still being concatenated.
    enum class CLexerInvalidationFlag : std::uint8_t {
        None          = 0,
        NoScan        = 1 << 0,
        NoMergeTokens = 1 << 1,
    };

    enum class TokenFlags : std::uint16_t {
        None             = 0,
        DanglingEscape   = 1 << 0,
        ExponentPending  = 1 << 1,
        AtLineStart      = 1 << 2,
        WhiteSpaceBefore = 1 << 3,
        SplicePending    = 1 << 4,
        CrPending        = 1 << 5,
        ContainsSplice   = 1 << 6,
        DirectiveLine    = 1 << 7,
        Skipped          = 1 << 8,
    };

};  // namespace Z::Zaban::Langs::CLang
namespace Z::Zaban {
    Z_ENABLE_BITMASK_OPERATORS(Langs::CLang::CLexerInvalidationFlag);
    Z_ENABLE_BITMASK_OPERATORS(Langs::CLang::TokenFlags);
}  // namespace Z::Zaban

namespace Z::Zaban::Langs::CLang {

    /// Removes '\' + newline runs. should only be called when
    /// ContainsSplice is true
    inline std::string unsplice(CLexerBufferType text) {
        std::string out;
        out.reserve(text.size());
        for (std::size_t i = 0; i < text.size(); ++i) {
            if ('\\' == text[i] && i + 1 < text.size()) {
                if ('\n' == text[i + 1]) {
                    ++i;
                    continue;
                }
                if ('\r' == text[i + 1]) {
                    ++i;
                    if (i + 1 < text.size() && '\n' == text[i + 1]) ++i;
                    continue;
                }
            }
            out.push_back(text[i]);
        }
        return out;
    }

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
        CLexerDiagnosticContext _diagnostics = CLexerDiagnosticContext();
        CLexerInternalState     _state       = CLexerInternalState::Normal;
        CLexerBufferType::const_iterator _buffer_it;
        std::vector<CLexerTokenType> _tokens  = std::vector<CLexerTokenType>();
        CLexerInvalidationFlag       _flags   = CLexerInvalidationFlag::None;
        TokenFlags                   _pending = TokenFlags::None;
        /// Absolute offset where the token under construction began.
        CLexerPositionType _token_start = 0;

        CLexerBufferType::const_pointer peek() const;
        CLexerBufferType::const_pointer peek(const CLexerPositionType) const;

        /// WARNING: nothing should touch _buffer_it outside advance()
        void advance();
        void advance(CLexerPositionType);
        void skip_splice();
        bool fold_line_ending();
        // A `/` sitting exactly on the seam may be the first half of `/*` or
        // `//`. Rewrite it into the matching open-comment anchor so repair()
        // this closes it against rhs, the same way it closes a comment body
        // that ran off the end of the chunk
        bool handle_split_cmt_opener(const CLexer& rhs);
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

        bool is_exponent_prefix(const char p) const {
            return (p == 'e' || p == 'E' || p == 'p' || p == 'P');
        }

       public:
        explicit CLexer(CLexerBufferType&);
        explicit CLexer(CLexerBufferType&, CLexerPositionType);

        void report(CLexerDiagnosticKind            kind,
                    OffsetRange<CLexerPositionType> range,
                    std::string_view                reason = {});

        CLexer& operator<<(const CLexer& rhs) {
            concat(rhs);
            return *this;
        }
        CLexer& operator<<(CLexer&& rhs) {
            concat(std::move(rhs));
            return *this;
        }

        [[nodiscard]]
        Lex::LexerDiagnosticContextBase& diagnostics() override {
            return this->_diagnostics;
        }
        void set_buffer(CLexerBufferType&) override;

        CLexerPositionType get_offset() override;
        void               set_offset(CLexerPositionType) override;

        bool                         scan() override;
        std::vector<CLexerTokenType> finalize() override;
    };
}  // namespace Z::Zaban::Langs::CLang
