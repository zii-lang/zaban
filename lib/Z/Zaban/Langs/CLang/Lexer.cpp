#include <Z/Zaban/Langs/CLang/Lexer.hpp>
#include <Z/Zaban/Langs/CLang/TokenKind.hpp>
#include <Z/Zaban/Lex/CharUtil.hpp>
#include <Z/Zaban/Lex/LexerError.hpp>
#include <Z/Zaban/SourcePosition.hpp>
#include <cstdint>
#include <memory>
#include <string_view>
#include <unordered_map>
#include <utility>

#include "Z/Zaban/BitmaskEnum.hpp"
#include "Z/Zaban/Langs/CLang/LexerTypes.hpp"

namespace Z::Zaban::Langs::CLang {
    const static std::unordered_map<std::string_view, CLexerTokenKind>
        CLangKeywords = {
            {"alignas", TokenKind::Alignas},
            {"_Alignas", TokenKind::Alignas},
            {"alignof", TokenKind::Alignof},
            {"_Alignof", TokenKind::Alignof},
            {"_Atomic", TokenKind::Atomic},
            {"auto", TokenKind::Auto},
            {"_BitInt", TokenKind::BitInt},
            {"bool", TokenKind::Bool},
            {"_Bool", TokenKind::Bool},
            {"break", TokenKind::Break},
            {"case", TokenKind::Case},
            {"char", TokenKind::Char},
            {"_Complex", TokenKind::Complex},
            {"const", TokenKind::Const},
            {"constexpr", TokenKind::Constexpr},
            {"continue", TokenKind::Continue},
            {"_Decimal32", TokenKind::Decimal32},
            {"_Decimal64", TokenKind::Decimal64},
            {"_Decimal128", TokenKind::Decimal128},
            {"default", TokenKind::Default},
            {"do", TokenKind::Do},
            {"double", TokenKind::Double},
            {"else", TokenKind::Else},
            {"enum", TokenKind::Enum},
            {"extern", TokenKind::Extern},
            {"false", TokenKind::False},
            {"float", TokenKind::Float},
            {"for", TokenKind::For},
            {"_Generic", TokenKind::Generic},
            {"goto", TokenKind::Goto},
            {"if", TokenKind::If},
            {"_Imaginary", TokenKind::Imaginary},
            {"inline", TokenKind::Inline},
            {"int", TokenKind::Int},
            {"long", TokenKind::Long},
            {"_Noreturn", TokenKind::Noreturn},
            {"nullptr", TokenKind::Nullptr},
            {"register", TokenKind::Register},
            {"restrict", TokenKind::Restrict},
            {"return", TokenKind::Return},
            {"short", TokenKind::Short},
            {"signed", TokenKind::Signed},
            {"sizeof", TokenKind::Sizeof},
            {"static", TokenKind::Static},
            {"static_assert", TokenKind::StaticAssert},
            {"_Static_assert", TokenKind::StaticAssert},
            {"struct", TokenKind::Struct},
            {"switch", TokenKind::Switch},
            {"thread_local", TokenKind::ThreadLocal},
            {"_Thread_local", TokenKind::ThreadLocal},
            {"true", TokenKind::True},
            {"typedef", TokenKind::Typedef},
            {"typeof", TokenKind::Typeof},
            {"typeof_unqual", TokenKind::TypeofUnqual},
            {"union", TokenKind::Union},
            {"unsigned", TokenKind::Unsigned},
            {"void", TokenKind::Void},
            {"volatile", TokenKind::Volatile},
            {"while", TokenKind::While},
    };

    CLexer::CLexer(CLexerBufferType& buffer) :
        Z::Zaban::Lex::Lexer<CLexerTokenType, CLexerPositionType,
                             CLexerBufferType>(buffer),
        _buffer_it(this->_buffer.begin()) {
    }

    CLexer::CLexer(CLexerBufferType& buffer, CLexerPositionType start_pos) :
        Z::Zaban::Lex::Lexer<CLexerTokenType, CLexerPositionType,
                             CLexerBufferType>(buffer, start_pos),
        _buffer_it(this->_buffer.begin()) {
    }

    void CLexer::set_buffer(CLexerBufferType& buffer) {
        this->_buffer    = buffer;
        this->_buffer_it = this->_buffer.begin();
    }

    CLexerPositionType CLexer::get_offset() {
        return this->_offset;
    }

    void CLexer::set_offset(CLexerPositionType offset) {
        this->_offset = offset;
    }

    void CLexer::invalidate(const CLexerInvalidationFlag flag) {
        this->_flags |= flag;
    }

    bool CLexer::has_flag(const CLexerInvalidationFlag flag) const {
        return (this->_flags & flag) == flag;
    }

    void CLexer::validate(const CLexerInvalidationFlag flag) {
        this->_flags &= ~flag;
    }

    bool CLexer::scan() {
        if (this->has_flag(CLexerInvalidationFlag::NoScan)) {
            return true;
        }
        this->_diagnostics.bump_scan();
        for (;;) {
            if (0 == this->_start_offset && this->_tokens.empty()) {
                this->_pending |= TokenFlags::AtLineStart;
            }
            this->skip_trivia();
            const CLexerBufferType::const_pointer p = this->peek();
            if (!p || this->eof() ||
                has(this->_pending, TokenFlags::SplicePending)) {
                // A trailing lone backslash leaves _buffer_it short of end():
                // not eof, and push_token() has already consumed
                // SplicePending. A null peek() is the only durable signal
                // left.
                this->_offset += static_cast<CLexerPositionType>(
                    this->_buffer.end() - this->_buffer_it);
                this->_buffer_it = this->_buffer.end();
                break;
            }

            const char c = *p;

            if (Lex::CharUtil::is_alpha(c) || '_' == c) {
                this->lex_ident_keyword();
            } else if (Lex::CharUtil::is_digit(c)) {
                this->lex_number();
            } else if ('.' == c) {
                const CLexerBufferType::const_pointer p1 = this->peek(1);
                if (p1 && Lex::CharUtil::is_digit(*p1)) {
                    this->lex_number();
                } else {
                    this->lex_punctuator();
                }
            } else if ('"' == c) {
                this->lex_string();
            } else if ('\'' == c) {
                this->lex_char();
            } else {
                this->lex_punctuator();
            }
        }

        if (any(this->_pending) && !this->_tokens.empty()) {
            this->_tokens.back().flags |=
                static_cast<std::uint8_t>(this->_pending);
            this->_pending = TokenFlags::None;
        }
        // Every scanned chunk is terminated by an end-of-buffer marker. concat
        // drops interior ones; the final Eob survives to the token stream.
        this->push_token(TokenKind::Eob, this->get_offset(),
                         this->get_offset());

        this->merge_double_tokens();
        return true;
    }

    std::vector<CLexerTokenType> CLexer::finalize() {
        // Any open fragment that survived every concat is a genuine
        // unterminated literal-> normalize its kind and record the error.
        for (auto& t: this->_tokens) {
            if (t.kind == TokenKind::StringOpen) {
                t.kind = TokenKind::String;
                this->set_error(CLexerErrorFlags::UnterminatedString);
            } else if (t.kind == TokenKind::CharOpen) {
                t.kind = TokenKind::CharLiteral;
                this->set_error(CLexerErrorFlags::UnterminatedCharLiteral);
            } else if (t.kind == TokenKind::DotDot) {
                t.kind = TokenKind::Dummy;
                this->set_error(CLexerErrorFlags::InvalidCharacter);
            } else if (t.kind == TokenKind::BlockCommentOpen ||
                       t.kind == TokenKind::LineCommentOpen) {
                this->set_error(CLexerErrorFlags::UnterminatedComment);
            }
        }
        std::erase_if(this->_tokens, [](const CLexerTokenType& t) {
            return t.kind == TokenKind::BlockCommentOpen ||
                   t.kind == TokenKind::LineCommentOpen;
        });
        return std::move(this->_tokens);
    }

    LexerDiagnostics& CLexer::diagnostics() {
        return _diagnostics;
    }

    void CLexer::lex_ident_keyword() {
        this->_token_start                           = this->get_offset();
        const CLexerBufferType::const_iterator begin = this->_buffer_it;
        for (;;) {
            const CLexerBufferType::const_pointer p = this->peek();
            if (!p) {
                break;
            }
            const char c = *p;
            if (!Lex::CharUtil::is_alpha(c) && !Lex::CharUtil::is_digit(c) &&
                '_' != c) {
                break;
            }
            this->advance();
        }
        const CLexerBufferType text(
            std::to_address(begin),
            static_cast<std::size_t>(this->_buffer_it - begin));

        CLexerTokenKind kind = CLexerTokenKind::Identifier;
        if (has(this->_pending, TokenFlags::ContainsSplice)) {
            const std::string clean = unsplice(text);
            const auto        it    = CLangKeywords.find(clean);
            if (it != CLangKeywords.end()) kind = it->second;
        } else {
            const auto it = CLangKeywords.find(text);
            if (it != CLangKeywords.end()) kind = it->second;
        }
        this->push_token(kind);
    }

    void CLexer::lex_number() {
        this->_token_start = this->get_offset();
        char prev          = 0;
        for (;;) {
            const CLexerBufferType::const_pointer p = this->peek();
            if (!p) {
                break;
            }
            const char c = *p;
            if (('+' == c || '-' == c) && this->is_exponent_prefix(prev)) {
                this->advance();
                prev = c;
                continue;
            }
            if (!Lex::CharUtil::is_alpha(c) && !Lex::CharUtil::is_digit(c) &&
                '.' != c) {
                break;
            }
            this->advance();
            prev = c;
        }
        this->push_token(CLexerTokenKind::Numeric);
        if (this->eof() && this->is_exponent_prefix(prev)) {
            this->_tokens.back().flags |=
                static_cast<std::uint8_t>(TokenFlags::ExponentPending);
        }
    }

    void CLexer::lex_string() {
        this->_token_start = this->get_offset();
        this->advance();  // opening "
        bool terminated = false;
        bool dangling   = false;
        for (;;) {
            const CLexerBufferType::const_pointer p = this->peek();
            if (!p) {
                break;
            }
            const char c = *p;
            if ('\\' == c) {
                this->advance();
                if (this->peek()) {
                    this->advance();
                } else {
                    // backslash escapes into the next chunk
                    dangling = true;
                }
                continue;
            }
            if ('"' == c) {
                this->advance();
                terminated = true;
                break;
            }
            if (Lex::CharUtil::is_linefeed(c)) {
                this->set_error(CLexerErrorFlags::UnterminatedString);
                terminated = true;
                break;
            }
            this->advance();
        }
        if (terminated) {
            this->push_token(CLexerTokenKind::String);
        } else {
            this->push_token(CLexerTokenKind::StringOpen);
            this->_tokens.back().flags |=
                static_cast<uint8_t>(TokenFlags::DanglingEscape);
        }
    }

    void CLexer::lex_char() {
        this->_token_start = this->get_offset();
        this->advance();  // opening '
        bool terminated = false;
        bool dangling   = false;
        for (;;) {
            const CLexerBufferType::const_pointer p = this->peek();
            if (!p) {
                break;
            }
            const char c = *p;
            if ('\\' == c) {
                this->advance();
                if (this->peek()) {
                    this->advance();
                } else {
                    dangling = true;
                }
                continue;
            }
            if ('\'' == c) {
                this->advance();
                terminated = true;
                break;
            }
            if (Lex::CharUtil::is_linefeed(c)) {
                this->set_error(CLexerErrorFlags::UnterminatedCharLiteral);
                terminated = true;
                break;
            }
            this->advance();
        }
        if (terminated) {
            this->push_token(CLexerTokenKind::CharLiteral);
        } else {
            this->push_token(CLexerTokenKind::CharOpen);
            this->_tokens.back().flags |=
                static_cast<uint8_t>(TokenFlags::DanglingEscape);
        }
    }
    CLexerPositionType CLexer::scan_comment_end_in_rhs(
        const CLexer& rhs, CLexerPositionType frag_begin) const {
        const auto&              buf  = rhs._buffer;
        const CLexerPositionType base = rhs._start_offset;

        // The `*/` may straddle the seam: `*` is the last byte of this chunk,
        // `/` the first of rhs. Only valid if that `*` isn't the opener's own.
        if (!buf.empty() && '/' == buf[0] && !this->_buffer.empty() &&
            '*' == this->_buffer.back() && base >= frag_begin + 3) {
            return base + 1;
        }

        for (std::size_t i = 0; i + 1 < buf.size(); ++i) {
            if ('*' == buf[i] && '/' == buf[i + 1]) {
                return base + i + 2;
            }
        }
        return CLexerPositionType(-1);
    }
    // Returns absolute offset one past the closing delimiter in rhs, or npos if
    // the delimiter never appears in rhs (literal spans >2 chunks stays
    // open). `dangling_escape` is true when the open fragment in `this` ended
    // on a lone backslash at the chunk boundary-> that backslash escapes rhs's
    // first byte, so we must skip it before scanning for the delimiter.
    CLexerPositionType CLexer::scan_in_rhs(const CLexer& rhs, char delim,
                                           bool dangling) const {
        const auto&              buf = rhs._buffer;
        const CLexerPositionType base =
            rhs._start_offset;  // abs offset of buf[0]
        std::size_t i = dangling ? 1 : 0;
        while (i < buf.size()) {
            const char c = buf[i];
            if ('\\' == c) {
                // skip escaped char. safe even if i+1 == size
                i += 2;
                continue;
            }
            if (delim == c) return base + i + 1;
            if (Lex::CharUtil::is_linefeed(c)) return CLexerPositionType(-1);
            ++i;
        }
        return CLexerPositionType(-1);
    }

    bool CLexer::repair(const CLexer&                 rhs,
                        std::vector<CLexerTokenType>& out_tail) {
        if (this->_tokens.empty()) return false;

        const TokenKind open_kind  = this->_tokens.back().kind;
        const bool      is_block   = (open_kind == TokenKind::BlockCommentOpen);
        const bool      is_line    = (open_kind == TokenKind::LineCommentOpen);
        const bool      is_comment = is_block || is_line;

        char      delim      = 0;
        TokenKind fused_kind = TokenKind::Dummy;
        if (open_kind == TokenKind::StringOpen) {
            delim      = '"';
            fused_kind = TokenKind::String;
        } else if (open_kind == TokenKind::CharOpen) {
            delim      = '\'';
            fused_kind = TokenKind::CharLiteral;
        } else if (!is_comment) {
            return false;
        }

        CLexerTokenType& open = this->_tokens.back();

        CLexerPositionType close_end;
        if (is_block) {
            close_end = this->scan_comment_end_in_rhs(rhs, open.range.begin);
        } else if (is_line) {
            close_end = this->scan_line_end_in_rhs(rhs);
        } else {
            close_end =
                this->scan_in_rhs(rhs, delim,
                                  has(static_cast<TokenFlags>(open.flags),
                                      TokenFlags::DanglingEscape));
        }

        out_tail.clear();

        if (close_end == CLexerPositionType(-1)) {
            // rhs doesn't close it. Extend across all of rhs, stay open for a
            // later chunk, keep the stream Eob-terminated.
            open.range.end = rhs._start_offset + rhs._buffer.size();
            out_tail.emplace_back(TokenKind::Eob,
                                  OffsetRange<CLexerPositionType>(
                                      open.range.end, open.range.end));
            return true;
        }

        if (is_comment) {
            this->_tokens.pop_back();  // trivia: contributes no token
        } else {
            open.kind      = fused_kind;
            open.range.end = close_end;
        }

        // Everything past the close was lexed by rhs under the wrong assumption
        // that it started outside a literal/comment. Re-lex it.
        CLexerBufferType tail_buf =
            rhs._buffer.substr(close_end - rhs._start_offset);
        CLexer tail_lexer(tail_buf, close_end);
        tail_lexer.scan();
        out_tail = std::move(tail_lexer._tokens);
        return true;
    }

    void CLexer::skip_trivia() {
        for (;;) {
            const CLexerBufferType::const_pointer p = this->peek();
            if (!p) {
                return;
            }
            const char c = *p;
            if (this->fold_line_ending()) {
                _pending |=
                    TokenFlags::AtLineStart | TokenFlags::WhiteSpaceBefore;
                continue;
            }
            if (Lex::CharUtil::is_whitespace(c)) {
                _pending |= TokenFlags::WhiteSpaceBefore;
                this->advance();
                continue;
            }
            const CLexerBufferType::const_pointer p1 = this->peek(1);
            if ('/' == c && p1 && '/' == *p1) {
                _pending |= TokenFlags::WhiteSpaceBefore;
                const CLexerPositionType start = this->get_offset();
                this->advance();
                this->advance();
                this->skip_line_comment_body(start);
                continue;
            }
            if ('/' == c && p1 && '*' == *p1) {
                _pending |= TokenFlags::WhiteSpaceBefore;
                const CLexerPositionType start = this->get_offset();
                this->advance();
                this->advance();
                this->skip_block_comment_body(start);
                continue;
            }
            break;
        }
    }

    void CLexer::skip_line_comment_body(CLexerPositionType start) {
        for (;;) {
            const CLexerBufferType::const_pointer p = this->peek();
            if (!p) {
                // Ran off the end without a newline. Emit an anchor so concat
                // can detect the seam falls inside a comment.
                this->push_token(TokenKind::LineCommentOpen, start,
                                 this->get_offset());
                return;
            }
            if (Lex::CharUtil::is_linefeed(*p)) {
                break;
            }
            this->advance();
        }
    }
    CLexerPositionType CLexer::scan_line_end_in_rhs(const CLexer& rhs) const {
        const auto& buf = rhs._buffer;
        for (std::size_t i = 0; i < buf.size(); ++i) {
            if (Lex::CharUtil::is_linefeed(buf[i])) {
                return rhs._start_offset + i;
            }
        }
        return CLexerPositionType(-1);
    }
    void CLexer::skip_block_comment_body(CLexerPositionType start) {
        for (;;) {
            const CLexerBufferType::const_pointer p = this->peek();
            if (!p) {
                // Ran off the end without `*/`. Emit an anchor so concat can
                // detect that the seam falls inside a comment. Comments are
                // trivia, so this token never survives to consumers.
                this->push_token(TokenKind::BlockCommentOpen, start,
                                 this->get_offset());
                return;
            }
            if ('*' == *p) {
                const CLexerBufferType::const_pointer p1 = this->peek(1);
                if (p1 && '/' == *p1) {
                    this->advance();
                    this->advance();
                    break;
                }
            }
            this->advance();
        }
    }

    // Each punctuator is emitted as its SHORTEST form here. The maximal-munch
    // combination (++ , >>= , -> , etc.) is performed by merge_double_tokens()
    // so that operators split across a chunk boundary are handled by exactly
    // the same rule as operators within one chunk. This mirrors Amir's ZLang
    // approach where "." + "." fuses to DDot only when offset-contiguous.
    void CLexer::lex_punctuator() {
        this->_token_start = this->get_offset();
        const char c       = *this->peek();
        this->advance();

        switch (c) {
            case '+':
                this->push_token(TokenKind::Plus);
                break;
            case '-':
                this->push_token(TokenKind::Minus);
                break;
            case '*':
                this->push_token(TokenKind::Asterisk);
                break;
            case '/':
                this->push_token(TokenKind::Slash);
                break;
            case '%':
                this->push_token(TokenKind::Percent);
                break;
            case '=':
                this->push_token(TokenKind::Equal);
                break;
            case '!':
                this->push_token(TokenKind::Exclam);
                break;
            case '~':
                this->push_token(TokenKind::Tilde);
                break;
            case '^':
                this->push_token(TokenKind::Caret);
                break;
            case '&':
                this->push_token(TokenKind::Amp);
                break;
            case '|':
                this->push_token(TokenKind::Pipe);
                break;
            case '<':
                this->push_token(TokenKind::Lesser);
                break;
            case '>':
                this->push_token(TokenKind::Greater);
                break;
            case '.':
                this->push_token(TokenKind::Dot);
                break;
            case ':':
                this->push_token(TokenKind::Colon);
                break;
            case '#':
                this->push_token(TokenKind::Hash);
                break;
            case '(':
                this->push_token(TokenKind::LParen);
                break;
            case ')':
                this->push_token(TokenKind::RParen);
                break;
            case '[':
                this->push_token(TokenKind::LBrak);
                break;
            case ']':
                this->push_token(TokenKind::RBrak);
                break;
            case '{':
                this->push_token(TokenKind::LBrace);
                break;
            case '}':
                this->push_token(TokenKind::RBrace);
                break;
            case ',':
                this->push_token(TokenKind::Comma);
                break;
            case ';':
                this->push_token(TokenKind::Semicolon);
                break;
            case '?':
                this->push_token(TokenKind::Question);
                break;
            default:
                this->set_error(CLexerErrorFlags::InvalidCharacter);
                this->push_token(TokenKind::Dummy);
                break;
        }
    }

    CLexerTokenKind CLexer::try_merge(const CLexerTokenType& a,
                                      const CLexerTokenType& b) const {
        // only fuse when b begins exactly where a ends. a.range.end is the
        // absolute offset one past a last byte. b.range.begin is b's first.
        if (a.range.end != b.range.begin) {
            return TokenKind::Dummy;
        }

        const TokenKind x = a.kind;
        const TokenKind y = b.kind;

        if (x == TokenKind::Identifier && y == TokenKind::Identifier) {
            return TokenKind::Identifier;
        }
        if (x == TokenKind::Identifier && y == TokenKind::Numeric) {
            // ident cannot be followed by a digit started fragment unless the
            // digits were part of the same identifier like foo|123 -> foo123.
            return TokenKind::Identifier;
        }
        if (x == TokenKind::Numeric &&
            has(static_cast<TokenFlags>(a.flags),
                TokenFlags::ExponentPending) &&
            (y == TokenKind::Minus || y == TokenKind::Plus)) {
            return TokenKind::Numeric;
        }
        if (x == TokenKind::Numeric &&
            (y == TokenKind::Numeric || y == TokenKind::Identifier ||
             y == TokenKind::Dot)) {
            // pp-number is greedy meaning 12|34, 1|e, 1|.5 all continue the
            // number.
            return TokenKind::Numeric;
        }
        if (x == TokenKind::Dot && y == TokenKind::Dot) {
            return TokenKind::DotDot;
        }
        if (x == TokenKind::Dot && y == TokenKind::DotDot) {
            return TokenKind::Ellipsis;
        }
        if (x == TokenKind::DotDot && y == TokenKind::Dot) {
            return TokenKind::Ellipsis;
        }
        switch (x) {
            case TokenKind::Plus:
                if (y == TokenKind::Plus) return TokenKind::PlusPlus;
                if (y == TokenKind::Equal) return TokenKind::PlusEqual;
                break;
            case TokenKind::Minus:
                if (y == TokenKind::Minus) return TokenKind::MinusMinus;
                if (y == TokenKind::Equal) return TokenKind::MinusEqual;
                if (y == TokenKind::Greater) return TokenKind::Arrow;
                break;
            case TokenKind::Asterisk:
                if (y == TokenKind::Equal) return TokenKind::AsteriskEqual;
                break;
            case TokenKind::Slash:
                if (y == TokenKind::Equal) return TokenKind::SlashEqual;
                break;
            case TokenKind::Percent:
                if (y == TokenKind::Equal) return TokenKind::PercentEqual;
                if (y == TokenKind::Colon) return TokenKind::Hash;
                if (y == TokenKind::Greater) return TokenKind::RBrace;
                break;
            case TokenKind::Caret:
                if (y == TokenKind::Equal) return TokenKind::CaretEqual;
                break;
            case TokenKind::Equal:
                if (y == TokenKind::Equal) return TokenKind::EqualEqual;
                break;
            case TokenKind::Exclam:
                if (y == TokenKind::Equal) return TokenKind::ExclamEqual;
                break;
            case TokenKind::Amp:
                if (y == TokenKind::Amp) return TokenKind::AmpAmp;
                if (y == TokenKind::Equal) return TokenKind::AmpEqual;
                break;
            case TokenKind::Pipe:
                if (y == TokenKind::Pipe) return TokenKind::PipePipe;
                if (y == TokenKind::Equal) return TokenKind::PipeEqual;
                break;
            case TokenKind::Lesser:
                if (y == TokenKind::Lesser) return TokenKind::LesserLesser;
                if (y == TokenKind::Equal) return TokenKind::LesserEqual;
                if (y == TokenKind::LesserEqual)
                    return TokenKind::LesserLesserEqual;
                if (y == TokenKind::Colon) return TokenKind::LBrak;
                if (y == TokenKind::Percent) return TokenKind::LBrace;
                break;
            case TokenKind::Greater:
                if (y == TokenKind::Greater) return TokenKind::GreaterGreater;
                if (y == TokenKind::Equal) return TokenKind::GreaterEqual;
                if (y == TokenKind::GreaterEqual)
                    return TokenKind::GreaterGreaterEqual;
                break;
            case TokenKind::Colon:
                if (y == TokenKind::Colon) return TokenKind::ColonColon;
                if (y == TokenKind::Greater) return TokenKind::RBrak;
                break;
            case TokenKind::Hash:
                if (y == TokenKind::Hash) return TokenKind::HashHash;
                break;
            case TokenKind::Dot:
                break;
            case TokenKind::LesserLesser:
                if (y == TokenKind::Equal) return TokenKind::LesserLesserEqual;
                break;
            case TokenKind::GreaterGreater:
                if (y == TokenKind::Equal)
                    return TokenKind::GreaterGreaterEqual;
                break;
            default:
                break;
        }

        return TokenKind::Dummy;
    }

    void CLexer::merge_double_tokens() {
        if (this->has_flag(CLexerInvalidationFlag::NoMergeTokens)) {
            return;
        }
        if (this->_tokens.size() < 2) {
            return;
        }

        bool changed = true;
        while (changed) {
            changed = false;
            std::vector<CLexerTokenType> out;
            out.reserve(this->_tokens.size());

            std::size_t i = 0;
            while (i < this->_tokens.size()) {
                if (i + 1 < this->_tokens.size()) {
                    const CLexerTokenType& a = this->_tokens[i];
                    const CLexerTokenType& b = this->_tokens[i + 1];
                    const bool a_ok = a.kind != TokenKind::Eob &&
                                      a.kind != TokenKind::Eof &&
                                      a.kind != TokenKind::StringOpen &&
                                      a.kind != TokenKind::CharOpen &&
                                      a.kind != TokenKind::BlockCommentOpen;
                    const bool b_ok = b.kind != TokenKind::Eob &&
                                      b.kind != TokenKind::Eof &&
                                      b.kind != TokenKind::StringOpen &&
                                      b.kind != TokenKind::CharOpen;
                    if (a_ok && b_ok) {
                        const TokenKind fused = this->try_merge(a, b);
                        if (fused != TokenKind::Dummy) {
                            out.emplace_back(fused,
                                             OffsetRange<CLexerPositionType>(
                                                 a.range.begin, b.range.end),
                                             a.flags | b.flags);
                            i += 2;
                            changed = true;
                            continue;
                        }
                    }
                }
                out.push_back(this->_tokens[i]);
                ++i;
            }
            this->_tokens = std::move(out);
        }
    }

    void CLexer::concat(const CLexer& rhs) {
        if (!this->_tokens.empty() &&
            this->_tokens.back().kind == TokenKind::Eob)
            this->_tokens.pop_back();

        this->handle_split_cmt_opener(rhs);

        std::vector<CLexerTokenType> tail;
        if (this->repair(rhs, tail)) {
            this->_tokens.insert(this->_tokens.end(),
                                 std::make_move_iterator(tail.begin()),
                                 std::make_move_iterator(tail.end()));
        } else {
            this->_tokens.insert(this->_tokens.end(), rhs._tokens.begin(),
                                 rhs._tokens.end());
        }

        this->_diagnostics.set_error(rhs._diagnostics.error());
        this->_diagnostics.bump_concat();
        this->merge_double_tokens();
    }

    void CLexer::concat(CLexer&& rhs) {
        if (!this->_tokens.empty() &&
            this->_tokens.back().kind == TokenKind::Eob)
            this->_tokens.pop_back();

        this->handle_split_cmt_opener(rhs);

        std::vector<CLexerTokenType> tail;
        if (this->repair(rhs, tail)) {
            this->_tokens.insert(this->_tokens.end(),
                                 std::make_move_iterator(tail.begin()),
                                 std::make_move_iterator(tail.end()));
        } else {
            this->_tokens.insert(this->_tokens.end(), rhs._tokens.begin(),
                                 rhs._tokens.end());
        }

        this->_diagnostics.set_error(rhs._diagnostics.error());
        this->_diagnostics.bump_concat();

        this->merge_double_tokens();
    }

    void CLexer::push_token(CLexerTokenKind token) {
        this->push_token(token, this->_token_start, this->get_offset());
    }

    void CLexer::push_token(CLexerTokenKind token, CLexerPositionType start,
                            CLexerPositionType end) {
        this->_tokens.emplace_back(token,
                                   OffsetRange<CLexerPositionType>(start, end),
                                   static_cast<std::uint8_t>(this->_pending));
        this->_pending = TokenFlags::None;
        this->_state   = CLexerInternalState::Normal;
    }

    bool CLexer::match_char(char ch) {
        const CLexerBufferType::const_pointer p = this->peek();
        if (!p || *p != ch) {
            return false;
        }
        this->advance();
        return true;
    }

    void CLexer::skip_splice() {
        while (_buffer_it != _buffer.end() && *_buffer_it == '\\') {
            auto p = _buffer_it + 1;
            if (p == _buffer.end()) {
                _pending |= TokenFlags::SplicePending;
                return;
            }
            if (*p == '\n') {
                _buffer_it = p + 1;
                _offset += 2;
            } else if (*p == '\r') {
                auto q = p + 1;
                if (q == _buffer.end()) {
                    _pending |=
                        TokenFlags::SplicePending | TokenFlags::CrPending;
                    return;
                }
                if (*q == '\n') {
                    _buffer_it = q + 1;
                    _offset += 3;
                } else {
                    _buffer_it = q;
                    _offset += 2;
                }
            } else {
                return;
            }
            _pending |= TokenFlags::ContainsSplice;
        }
    }

    bool CLexer::handle_split_cmt_opener(const CLexer& rhs) {
        if (this->_tokens.empty()) return false;

        CLexerTokenType& back = this->_tokens.back();
        if (back.kind != TokenKind::Slash) return false;
        if (back.range.end != rhs._start_offset) return false;

        // rhs may open with its own splices: `/` + `\<LF>*` is still `/*`.
        const auto& buf = rhs._buffer;
        std::size_t i   = 0;
        while (i < buf.size() && '\\' == buf[i]) {
            if (i + 1 >= buf.size()) return false;
            if ('\n' == buf[i + 1]) {
                i += 2;
            } else if ('\r' == buf[i + 1]) {
                i += (i + 2 < buf.size() && '\n' == buf[i + 2]) ? 3 : 2;
            } else {
                return false;
            }
        }
        if (i >= buf.size()) return false;

        if ('*' == buf[i]) {
            back.kind = TokenKind::BlockCommentOpen;
            return true;
        }
        if ('/' == buf[i]) {
            back.kind = TokenKind::LineCommentOpen;
            return true;
        }
        return false;
    }

    void CLexer::advance() {
        this->advance(1);
    }

    void CLexer::advance(CLexerPositionType offset) {
        const auto remaining = static_cast<CLexerPositionType>(
            this->_buffer.end() - this->_buffer_it);
        if (offset > remaining) {
            set_error(CLexerErrorFlags::UnexpectedEndOfFile);
            offset = remaining;
        }
        this->_offset += offset;
        this->_buffer_it += offset;
        skip_splice();
    }

    bool CLexer::eof() const {
        return (this->_buffer_it >= this->_buffer.end());
    }

    CLexerBufferType::const_pointer CLexer::peek() const {
        return this->peek(0);
    }
    CLexerBufferType::const_pointer CLexer::peek(
        CLexerPositionType offset) const {
        auto it = this->_buffer_it;
        for (;;) {
            while (it < this->_buffer.end() && '\\' == *it) {
                const auto p = it + 1;
                if (p == this->_buffer.end()) return nullptr;
                if ('\n' == *p) {
                    it = p + 1;
                    continue;
                }
                if ('\r' == *p) {
                    const auto q = p + 1;
                    if (q == this->_buffer.end()) return nullptr;
                    it = ('\n' == *q) ? q + 1 : q;
                    continue;
                }
                break;
            }
            if (it >= this->_buffer.end()) return nullptr;
            if (0 == offset) return std::to_address(it);
            --offset;
            ++it;
        }
    }
    std::string CLexer::unsplice(CLexerBufferType text) {
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
    bool CLexer::fold_line_ending() {
        const CLexerBufferType::const_pointer p = this->peek();
        if (!p) return false;
        if ('\n' == *p) {
            this->advance();
            return true;
        }
        if ('\r' == *p) {
            this->advance();
            const CLexerBufferType::const_pointer q = this->peek();
            if (q && '\n' == *q)
                this->advance();
            else if (!q)
                _pending |= TokenFlags::CrPending;
            return true;
        }
        return false;
    }

}  // namespace Z::Zaban::Langs::CLang
