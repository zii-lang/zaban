#include <Z/Zaban/Langs/CLang/Lexer.hpp>
#include <memory>
#include <string_view>
#include <unordered_map>
#include <utility>

#include "Z/Zaban/Langs/CLang/TokenKind.hpp"
#include "Z/Zaban/Lex/CharUtil.hpp"
#include "Z/Zaban/Lex/LexerDiagnostics.hpp"
#include "Z/Zaban/SourcePosition.hpp"

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
        this->_flags &= flag;
    }

    bool CLexer::scan() {
        if (this->has_flag(CLexerInvalidationFlag::NoScan)) {
            return true;
        }

        for (;;) {
            this->skip_trivia();
            if (this->eof()) {
                break;
            }

            const char c = *this->peek();

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
                if (this->_error == CLexerError::None) {
                    this->_error = CLexerError::UnterminatedString;
                }
            } else if (t.kind == TokenKind::CharOpen) {
                t.kind = TokenKind::CharLiteral;
                if (this->_error == CLexerError::None) {
                    this->_error = CLexerError::UnterminatedCharLiteral;
                }
            }
        }
        return std::move(this->_tokens);
    }

    LexerDiagnostics& CLexer::diagnostics() {
        // TODO: surface _error once LexerDiagnostics carries payload.
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
        const auto it = CLangKeywords.find(text);
        this->push_token(it != CLangKeywords.end()
                             ? it->second
                             : CLexerTokenKind::Identifier);
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
                this->set_error(CLexerError::UnterminatedString);
                terminated = true;
                break;
            }
            this->advance();
        }
        if (terminated) {
            this->push_token(CLexerTokenKind::String);
        } else {
            this->push_token(CLexerTokenKind::StringOpen);
            this->_tokens.back().dangling_escape = dangling;
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
                this->set_error(CLexerError::UnterminatedCharLiteral);
                terminated = true;
                break;
            }
            this->advance();
        }
        if (terminated) {
            this->push_token(CLexerTokenKind::CharLiteral);
        } else {
            this->push_token(CLexerTokenKind::CharOpen);
            this->_tokens.back().dangling_escape = dangling;
        }
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

    std::size_t CLexer::repair(const CLexer& rhs) {
        if (this->_tokens.empty()) return 0;
        CLexerTokenType& open = this->_tokens.back();

        char      delim;
        TokenKind fused_kind;
        if (open.kind == TokenKind::StringOpen) {
            delim      = '"';
            fused_kind = TokenKind::String;
        } else if (open.kind == TokenKind::CharOpen) {
            delim      = '\'';
            fused_kind = TokenKind::CharLiteral;
        } else {
            return 0;
        }

        const CLexerPositionType close_end =
            this->scan_in_rhs(rhs, delim, open.dangling_escape);

        if (close_end == CLexerPositionType(-1)) {
            // rhs does not close the literal. extends the fragment across all
            // of rhs and keep it open for a later chunk to close. eats every
            // rhs token EXCEPT a trailing Eob so the stream stays terminated
            open.range.end = rhs._start_offset + rhs._buffer.size();
            std::size_t n  = rhs._tokens.size();
            if (n > 0 && rhs._tokens[n - 1].kind == TokenKind::Eob) {
                --n;
            }
            return n;
        }

        open.kind      = fused_kind;
        open.range.end = close_end;

        std::size_t k = 0;
        while (k < rhs._tokens.size() &&
               rhs._tokens[k].range.begin < close_end) {
            ++k;
        }
        // swallow first k rhs tokens (the literal body)
        return k;
    }

    void CLexer::skip_trivia() {
        for (;;) {
            const CLexerBufferType::const_pointer p = this->peek();
            if (!p) {
                return;
            }
            const char c = *p;
            if (Lex::CharUtil::is_whitespace(c) ||
                Lex::CharUtil::is_linefeed(c)) {
                this->advance();
                continue;
            }
            const CLexerBufferType::const_pointer p1 = this->peek(1);
            if ('/' == c && p1 && '/' == *p1) {
                this->advance();
                this->advance();
                this->skip_line_comment_body();
                continue;
            }
            if ('/' == c && p1 && '*' == *p1) {
                this->advance();
                this->advance();
                this->skip_block_comment_body();
                continue;
            }
            break;
        }
    }

    void CLexer::skip_line_comment_body() {
        for (;;) {
            const CLexerBufferType::const_pointer p = this->peek();
            if (!p) {
                return;
            }
            if (Lex::CharUtil::is_linefeed(*p)) {
                break;
            }
            this->advance();
        }
    }

    void CLexer::skip_block_comment_body() {
        for (;;) {
            const CLexerBufferType::const_pointer p = this->peek();
            if (!p) {
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
                this->set_error(CLexerError::InvalidCharacter);
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
            (y == TokenKind::Numeric || y == TokenKind::Identifier ||
             y == TokenKind::Dot)) {
            // pp-number is greedy meaning 12|34, 1|e, 1|.5 all continue the
            // number.
            return TokenKind::Numeric;
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
                break;
            case TokenKind::Greater:
                if (y == TokenKind::Greater) return TokenKind::GreaterGreater;
                if (y == TokenKind::Equal) return TokenKind::GreaterEqual;
                break;
            case TokenKind::Colon:
                if (y == TokenKind::Colon) return TokenKind::ColonColon;
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
                                      a.kind != TokenKind::CharOpen;
                    const bool b_ok = b.kind != TokenKind::Eob &&
                                      b.kind != TokenKind::Eof &&
                                      b.kind != TokenKind::StringOpen &&
                                      b.kind != TokenKind::CharOpen;
                    if (a_ok && b_ok) {
                        const TokenKind fused = this->try_merge(a, b);
                        if (fused != TokenKind::Dummy) {
                            out.emplace_back(
                                fused, SourcePositionRange<CLexerPositionType>(
                                           a.range.begin, b.range.end));
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
        const std::size_t skip = this->repair(rhs);
        this->_tokens.insert(this->_tokens.end(), rhs._tokens.begin() + skip,
                             rhs._tokens.end());
        if (rhs._error != CLexerError::None && _error == CLexerError::None)
            this->_error = rhs._error;
        this->merge_double_tokens();
    }

    void CLexer::concat(CLexer&& rhs) {
        if (!this->_tokens.empty() &&
            this->_tokens.back().kind == TokenKind::Eob)
            this->_tokens.pop_back();
        const std::size_t skip = this->repair(rhs);
        this->_tokens.insert(
            this->_tokens.end(),
            std::make_move_iterator(rhs._tokens.begin() + skip),
            std::make_move_iterator(rhs._tokens.end()));
        if (rhs._error != CLexerError::None && _error == CLexerError::None)
            this->_error = rhs._error;
        this->merge_double_tokens();
    }

    void CLexer::push_token(CLexerTokenKind token) {
        this->push_token(token, this->_token_start, this->get_offset());
    }

    void CLexer::push_token(CLexerTokenKind token, CLexerPositionType start,
                            CLexerPositionType end) {
        this->_tokens.emplace_back(
            token, SourcePositionRange<CLexerPositionType>(start, end));
        this->_state = CLexerInternalState::Normal;
    }

    bool CLexer::match_char(char ch) {
        const CLexerBufferType::const_pointer p = this->peek();
        if (!p || *p != ch) {
            return false;
        }
        return this->advance();
    }

    bool CLexer::advance() {
        return this->advance(1);
    }

    bool CLexer::advance(CLexerPositionType offset) {
        if (offset > static_cast<CLexerPositionType>(this->_buffer.end() -
                                                     this->_buffer_it)) {
            return false;
        }
        this->_offset += offset;
        this->_buffer_it += offset;
        return true;
    }

    bool CLexer::eof() const {
        return (this->_buffer_it >= this->_buffer.end());
    }

    CLexerBufferType::const_pointer CLexer::peek() const {
        return this->peek(0);
    }

    CLexerBufferType::const_pointer CLexer::peek(
        const CLexerPositionType offset) const {
        if (offset >= static_cast<CLexerPositionType>(this->_buffer.end() -
                                                      this->_buffer_it)) {
            return nullptr;
        }
        return std::to_address(this->_buffer_it + offset);
    }

}  // namespace Z::Zaban::Langs::CLang
