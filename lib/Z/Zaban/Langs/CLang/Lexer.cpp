#include <Z/Zaban/Langs/CLang/Lexer.hpp>
#include <memory>
#include <string_view>
#include <unordered_map>

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

    void CLexer::set_buffer(CLexerBufferType& buffer) {
        // Account for any bytes of the outgoing chunk that were never
        // consumed. Usually 0. non-zero only on an early stop. Keeping the
        // absolute offset correct means token ranges stay meaningful across
        // chunks even though the buffers themselves are not!
        this->_offset += static_cast<CLexerPositionType>(this->_buffer.end() -
                                                         this->_buffer_it);

        this->_buffer    = buffer;
        this->_buffer_it = this->_buffer.begin();
    }

    // TODO:
    bool CLexer::analyze() {
        return false;
    }

    std::vector<CLexerTokenType> CLexer::finalize() {
        return std::move(this->_tokens);
    }

    LexerDiagnostics CLexer::diagnostics() {
        // TODO:
        return LexerDiagnostics();
    }

    void CLexer::suspend(CLexerInternalState resume_state) {
        // append everything consumed in 'this' chunk for the current token
        // to the pending buffer and then record where to pick backup. on the
        // nxt chunk, resume() replayes resume_state and keeps appending!
        this->_pending.append(this->_buffer.begin(),
                              std::to_address(this->_buffer_it));
        this->_state = resume_state;
    }

    void CLexer::resume() {
        using CIS = CLexerInternalState;
        switch (this->_state) {
            case CIS::Ident:
                this->lex_ident_body();
                break;
            case CIS::Number:
                this->lex_number_body();
                break;
            case CIS::String:
                this->lex_string_body();
                break;
            case CIS::CharLiteral:
                this->lex_char_body();
                break;
            case CIS::LineComment:
                this->skip_line_comment_body();
                break;
            case CIS::BlockComment:
                this->skip_block_comment_body();
                break;
            case CIS::Normal:
                break;
        }
    }

    void CLexer::lex_ident_body() {
        for (;;) {
            CLexerBufferType::const_pointer p = this->peek();
            if (!p) {
                this->suspend(CLexerInternalState::Ident);
                return;
            }
            if (!Lex::CharUtil::is_alpha(*p) && !Lex::CharUtil::is_digit(*p) &&
                '_' != *p) {
                break;
            }
            this->advance();
        }
        const CLexerBufferType text = this->current_lexeme();
        const auto             it   = CLangKeywords.find(text);
        this->push_token(it != CLangKeywords.end()
                             ? it->second
                             : CLexerTokenKind::Identifier);
    }
    void CLexer::lex_ident_keyword() {
        this->_token_start = this->get_offset();
        this->lex_ident_body();
    }

    /// pp-number: greedy. prev lets us keep sign chars that follow an
    /// exponent marker (1e+5) without swallowing a stray binary minus.
    /// On resume, prev restarts at 0, which is safe bcuz a boundary landing
    /// between the exponent marker and its sign is vanishingly rare and
    /// at worst ends the number one char early, which the parser should reject.
    void CLexer::lex_number_body() {
        char prev = 0;
        for (;;) {
            CLexerBufferType::const_pointer p = this->peek();
            if (!p) {
                this->suspend(CLexerInternalState::Number);
                return;
            }
            const char c = *p;
            if (('+' == c || '-' == c) && this->is_exponent_prefix(prev)) {
                this->advance();
                prev = c;
                continue;
            }
            if (!Lex::CharUtil::is_alpha(*p) && !Lex::CharUtil::is_digit(*p) &&
                '-' != *p && '.' != *p) {
                break;
            }

            this->advance();
            prev = c;
        }
        this->push_token(CLexerTokenKind::Numeric);
    }
    void CLexer::lex_number() {
        this->_token_start = this->get_offset();
        this->lex_number_body();
    }

    void CLexer::lex_string() {
        this->_token_start = this->get_offset();
        // "
        this->advance();
        this->lex_string_body();
    }
    void CLexer::lex_string_body() {
        for (;;) {
            const CLexerBufferType::const_pointer p = this->peek();
            if (!p) {
                this->suspend(CLexerInternalState::String);
                return;
            }
            const char c = *p;
            if ('\\' == c) {
                if (!this->peek(1)) {
                    // for backslash itself
                    this->advance();
                    this->suspend(CLexerInternalState::String);
                    return;
                }
                // for backslash itself
                this->advance();
                // for the esc char
                this->advance();
                continue;
            }

            if ('"' == c) {
                this->advance();
                break;
            }
            if (Lex::CharUtil::is_linefeed(c)) {
                this->set_error(CLexerError::UnterminatedString);
                break;
            }
            this->advance();
        }

        this->push_token(CLexerTokenKind::String);
    }

    void CLexer::lex_char() {
        this->_token_start = this->get_offset();
        // '
        this->advance();
        this->lex_char_body();
    }

    void CLexer::lex_char_body() {
        for (;;) {
            const CLexerBufferType::const_pointer p = this->peek();
            if (!p) {
                this->suspend(CLexerInternalState::CharLiteral);
                return;
            }
            const auto c = *p;
            if ('\'' == c) {
                this->advance();
                break;
            }
            if ('\\' == c) {
                if (!this->peek()) {
                    this->advance();
                    this->suspend(CLexerInternalState::CharLiteral);
                    return;
                }
                this->advance();
                this->advance();
                continue;
            }
            if (Lex::CharUtil::is_linefeed(c)) {
                this->set_error(CLexerError::UnterminatedCharLiteral);
                break;
            }
            this->advance();
        }
        this->push_token(CLexerTokenKind::CharLiteral);
    }

    void CLexer::skip_line_comment_body() {
        for (;;) {
            const CLexerBufferType::const_pointer p = this->peek();
            if (!p) {
                // no need to save anything! we dont care about the comments
                this->_state = CLexerInternalState::LineComment;
                return;
            }
            if (Lex::CharUtil::is_linefeed(*p)) {
                break;
            }
            this->advance();
        }
        this->_state = CLexerInternalState::Normal;
    }
    void CLexer::skip_block_comment_body() {
        for (;;) {
            const CLexerBufferType::const_pointer p = this->peek();
            if (!p) {
                this->_state = CLexerInternalState::BlockComment;
                return;
            }
            if ('*' == *p) {
                const CLexerBufferType::const_pointer q = this->peek(1);
                if (!q) {
                    this->_state = CLexerInternalState::BlockComment;
                    return;
                }
                if ('/' == *q) {
                    this->advance();  //*
                    this->advance();  // /
                }
            }
            this->advance();
        }
        this->_state = CLexerInternalState::Normal;
    }

    // TODO: these can appear inside any token in C and are usually handled
    // by the preprocessor (per stallman's article).
    void CLexer::skip_line_continuations() {
    }

    /// whitespace + comments
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
            const CLexerBufferType::const_pointer q = this->peek(1);
            if ('/' == c && q && '/' == *q) {
                this->advance();
                this->advance();
                this->skip_line_comment_body();
                if (this->_state != CLexerInternalState::Normal) {
                    // cmnt ran off the chunk end
                    return;
                }
                continue;
            }
            if ('/' == c && q && '*' == *q) {
                this->advance();
                this->advance();
                this->skip_block_comment_body();
                if (this->_state != CLexerInternalState::Normal) {
                    return;
                }
                continue;
            }
            // a '/' with no flwing char yet means we can't tell if it's a
            // comment or not. the punctuator/comment will decide on
            // the next chunk.
            // analyze() will re-enter skip_trivia then.
            if ('/' == c && !q) {
                return;
            }
            // if it reaches here, real tokens have been met!
            break;
        }
    }

    void CLexer::lex_punctuator() {
        this->_token_start = this->get_offset();
        const char c       = *this->peek();
        this->advance();
        switch (c) {
            case '+':
                if (this->match_char('+')) {
                    this->push_token(TokenKind::PlusPlus);
                } else if (this->match_char('=')) {
                    this->push_token(TokenKind::PlusEqual);
                } else {
                    this->push_token(TokenKind::Plus);
                }
                break;
            case '-':
                if (this->match_char('-')) {
                    this->push_token(TokenKind::MinusMinus);
                } else if (this->match_char('=')) {
                    this->push_token(TokenKind::MinusEqual);
                } else if (this->match_char('>')) {
                    this->push_token(TokenKind::Arrow);
                } else {
                    this->push_token(TokenKind::Minus);
                }
                break;
            case '*':
                this->push_token(this->match_char('=')
                                     ? TokenKind::AsteriskEqual
                                     : TokenKind::Asterisk);
                break;
            case '/':
                this->push_token(this->match_char('=') ? TokenKind::SlashEqual
                                                       : TokenKind::Slash);
                break;
            case '%':
                this->push_token(this->match_char('=') ? TokenKind::PercentEqual
                                                       : TokenKind::Percent);
                break;
            case '=':
                this->push_token(this->match_char('=') ? TokenKind::EqualEqual
                                                       : TokenKind::Equal);
                break;
            case '!':
                this->push_token(this->match_char('=') ? TokenKind::ExclamEqual
                                                       : TokenKind::Exclam);
                break;
            case '^':
                this->push_token(this->match_char('=') ? TokenKind::CaretEqual
                                                       : TokenKind::Caret);
                break;
            case '~':
                this->push_token(TokenKind::Tilde);
                break;
            case '&':
                if (this->match_char('&')) {
                    this->push_token(TokenKind::AmpAmp);
                } else if (this->match_char('=')) {
                    this->push_token(TokenKind::AmpEqual);
                } else {
                    this->push_token(TokenKind::Amp);
                }
                break;
            case '|':
                if (this->match_char('|')) {
                    this->push_token(TokenKind::PipePipe);
                } else if (this->match_char('=')) {
                    this->push_token(TokenKind::PipeEqual);
                } else {
                    this->push_token(TokenKind::Pipe);
                }
                break;
            case '<':
                if (this->match_char('<')) {
                    this->push_token(this->match_char('=')
                                         ? TokenKind::LesserLesserEqual
                                         : TokenKind::LesserLesser);
                } else if (this->match_char('=')) {
                    this->push_token(TokenKind::LesserEqual);
                } else {
                    this->push_token(TokenKind::Lesser);
                }
                break;
            case '>':
                if (this->match_char('>')) {
                    this->push_token(this->match_char('=')
                                         ? TokenKind::GreaterGreaterEqual
                                         : TokenKind::GreaterGreater);
                } else if (this->match_char('=')) {
                    this->push_token(TokenKind::GreaterEqual);
                } else {
                    this->push_token(TokenKind::Greater);
                }
                break;
            case '.':
                if (this->peek() && '.' == *this->peek() && this->peek(1) &&
                    '.' == *this->peek(1)) {
                    this->advance();
                    this->advance();
                    this->push_token(TokenKind::Ellipsis);
                } else {
                    this->push_token(TokenKind::Dot);
                }
                break;
            case ':':
                this->push_token(this->match_char(':') ? TokenKind::ColonColon
                                                       : TokenKind::Colon);
                break;
            case '#':
                this->push_token(this->match_char('#') ? TokenKind::HashHash
                                                       : TokenKind::Hash);
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

    // NOTE: if _pending is not empty, returns a view over _pending, valid until
    // the next suspend() or push_token().
    CLexerBufferType CLexer::current_lexeme() {
        CLexerPositionType in_chunk =
            std::to_address(this->_buffer_it) - this->_buffer.begin();
        if (this->_pending.empty()) {
            return this->_buffer.substr(0, 0).empty()
                       ? CLexerBufferType(this->_buffer.data(), 0)
                       : CLexerBufferType(this->_buffer.data(), in_chunk);
        }
        this->_pending.append(this->_buffer.begin(),
                              std::to_address(this->_buffer_it));
        return CLexerBufferType(this->_pending);
    }

    void CLexer::push_token(CLexerTokenKind token) {
        this->_tokens.emplace_back(
            token, SourcePositionRange<CLexerPositionType>(this->_token_start,
                                                           this->get_offset()));
        this->_state = CLexerInternalState::Normal;
        this->_pending.clear();
    }

    bool CLexer::match_char(char ch) {
        CLexerBufferType::const_pointer p = this->peek();
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
                                                     this->_buffer_it))
            return false;

        this->_offset += offset;
        this->_buffer_it += offset;
        return true;
    }

    CLexerPositionType CLexer::get_offset() {
        return this->_offset;
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

    CLexerBufferType::const_pointer CLexer::get() {
        CLexerBufferType::const_pointer ch = this->peek();
        if (ch != nullptr) {
            advance();
        }
        return ch;
    }

}  // namespace Z::Zaban::Langs::CLang
