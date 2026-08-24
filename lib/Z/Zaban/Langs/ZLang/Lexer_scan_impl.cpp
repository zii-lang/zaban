// System imports
#include <unordered_map>
// Z imports
#include <Z/Zaban/Langs/ZLang/Lexer.hpp>
#include <Z/Zaban/Lex/ScanUtil.hpp>

namespace Z::Zaban::Langs::ZLang {
    static bool scan_whitespace_or_newline(ZLexer& lexer) {
        const auto* p0 = lexer.peek();

        if (p0 == nullptr) {
            return false;
        }

        if (Lex::CharUtil::is_whitespace(*p0)) {
            lexer.advance();
            return true;
        }

        if (!Lex::CharUtil::is_linefeed(*p0)) {
            return false;
        }

        const auto* p1 = lexer.peek(1);

        if (p1 == nullptr) {
            lexer.advance();
            return true;
        }

        ZLexerPositionType line_char_count = 0;

        if (Lex::ScanUtil::is_newline_seq(*p0, *p1, &line_char_count)) {
            lexer.advance(line_char_count);

        } else {
            lexer.advance();
        }

        return true;
    }

    static ZLexerSkipResult scan_line_comment(ZLexer& lexer) {
        lexer.set_state(ZLexerInternalState::LineComment);

        while (const auto* p0 = lexer.peek()) {
            if (Lex::CharUtil::is_linefeed(*p0)) {
                lexer.set_state(ZLexerInternalState::Normal);

                // Do not consume the newline here.
                return ZLexerSkipResult::NonTrivial;
            }

            lexer.advance();
        }

        // EOF terminates a line comment successfully.
        lexer.set_state(ZLexerInternalState::Normal);

        return ZLexerSkipResult::EndOfInput;
    }

    static ZLexerSkipResult scan_block_comment(ZLexer& lexer) {
        lexer.set_state(ZLexerInternalState::BlockComment);

        while (true) {
            const auto* p0 = lexer.peek();

            if (p0 == nullptr) {
                return ZLexerSkipResult::Incomplete;
            }

            const auto* p1 = lexer.peek(1);

            if (p1 != nullptr && *p0 == '*' && *p1 == '/') {
                lexer.advance(2);

                lexer.set_state(ZLexerInternalState::Normal);

                return ZLexerSkipResult::NonTrivial;
            }

            lexer.advance();
        }
    }

    static ZLexerSkipResult scan_comment(ZLexer& lexer) {
        const auto* p0 = lexer.peek();

        if (p0 == nullptr) {
            return ZLexerSkipResult::EndOfInput;
        }

        const auto* p1 = lexer.peek(1);

        if (p1 == nullptr ||
            !Lex::ScanUtil::is_either_slash_comment(*p0, *p1)) {
            return ZLexerSkipResult::NonTrivial;
        }

        const bool is_line_comment =
            Lex::ScanUtil::is_double_slash_comment(*p0, *p1);

        // Consume // or /*
        lexer.advance(2);

        if (is_line_comment) {
            return scan_line_comment(lexer);
        }

        return scan_block_comment(lexer);
    }

    ZLexerSkipResult ZLexer::skip_trivial() {
        //
        // Resume an incomplete trivia construct.
        //
        switch (this->_state) {
            case ZLexerInternalState::LineComment: {
                const auto result = scan_line_comment(*this);

                if (result == ZLexerSkipResult::Incomplete ||
                    result == ZLexerSkipResult::EndOfInput) {
                    return result;
                }

                break;
            }

            case ZLexerInternalState::BlockComment: {
                const auto result = scan_block_comment(*this);

                if (result != ZLexerSkipResult::NonTrivial) {
                    return result;
                }

                break;
            }

            default:
                break;
        }

        //
        // Normal trivia scanning.
        //
        while (true) {
            if (this->peek() == nullptr) {
                return ZLexerSkipResult::EndOfInput;
            }

            if (scan_whitespace_or_newline(*this)) {
                continue;
            }

            const auto comment_result = scan_comment(*this);

            switch (comment_result) {
                case ZLexerSkipResult::NonTrivial:
                    return ZLexerSkipResult::NonTrivial;

                case ZLexerSkipResult::Incomplete:
                    return ZLexerSkipResult::Incomplete;

                case ZLexerSkipResult::EndOfInput:
                    return ZLexerSkipResult::EndOfInput;
            }
        }
    }

    ScanResult ZLexer::scan_impl() {
        auto add_token = [this](ZLexerTokenKind kind, ZLexerPositionType start,
                                ZLexerPositionType end) {
            this->_tokens.emplace_back(
                kind, OffsetRange<ZLexerPositionType>(start, end));
        };

        auto scan_string = [this, &add_token](
                               ZLexerBufferType::value_type quote,
                               ZLexerPositionType           start) {
            add_token(ZLexerTokenKind::String, start, start);
            this->_state = quote == '\'' ? ZLexerInternalState::SQString
                                         : ZLexerInternalState::DQString;
            this->advance();
            // return this->scan_until_eos();
            return false;
        };

        auto scan_identifier = [this, &add_token](ZLexerPositionType start) {
            std::string text;

            while (const auto* p = this->peek()) {
                if (!is_identifier_continue(*p)) {
                    break;
                }

                text.push_back(*p);
                this->advance();
            }

            // The identifier is incomplete from the perspective of
            // incremental lexing. Do NOT classify it yet.
            //
            // Example:
            //
            //     "ret" + "urn"
            //
            // `ret` must remain Identifier until we see what the
            // next chunk contributes.
            if (this->peek() == nullptr) {
                this->_state = ZLexerInternalState::Identifier;

                add_token(ZLexerTokenKind::Identifier, start,
                          this->_offset - 1);

                return true;
            }

            this->_state = ZLexerInternalState::Normal;

            add_token(classify_identifier(text), start, this->_offset - 1);

            return true;
        };

        auto scan_number = [this, &add_token](ZLexerPositionType start) {
            const auto first = *this->peek();
            this->advance();
            this->_state = first == '0' ? ZLexerInternalState::ZeroStart
                                        : ZLexerInternalState::Number;

            // if (!this->scan_until_get_numeric()) {
            return false;
            // }

            add_token(ZLexerTokenKind::Numeric, start, this->_offset - 1);
            return true;
        };

        if (this->eob()) {
            return ScanResult::EndOfInput;
        }
        this->_dc.record_scan();

        if (this->_state != ZLexerInternalState::Normal) {
            switch (this->_state) {
                case ZLexerInternalState::LineComment:
                    // if (!this->scan_double_slash_close_comment()) {
                    return ScanResult::Incomplete;
                    // }
                    break;

                case ZLexerInternalState::BlockComment:
                    // if (!this->scan_until_block_slash_close_comment()) {
                    return ScanResult::Incomplete;
                    // }
                    break;

                case ZLexerInternalState::SQString:
                case ZLexerInternalState::DQString:
                    // if (!this->scan_until_eos()) {
                    return ScanResult::Incomplete;
                    // }
                    break;

                case ZLexerInternalState::Identifier: {
                    const auto continuation_start = this->_offset;

                    std::string continuation;

                    while (const auto* p = this->peek()) {
                        if (!is_identifier_continue(*p)) {
                            break;
                        }

                        continuation.push_back(*p);
                        this->advance();
                    }

                    // The next chunk did not actually continue the
                    // identifier.
                    if (this->_offset == continuation_start) {
                        this->_state = ZLexerInternalState::Normal;
                        break;
                    }

                    add_token(ZLexerTokenKind::Identifier, continuation_start,
                              this->_offset - 1);
                    // Still potentially continued by another chunk.
                    if (this->peek() == nullptr) {
                        this->_state = ZLexerInternalState::Identifier;
                        return ScanResult::EndOfInput;
                    } else {
                        this->_state = ZLexerInternalState::Normal;
                    }
                    break;
                }
                default:
                    if (this->_state > ZLexerInternalState::STATE_NumStart &&
                        this->_state < ZLexerInternalState::STATE_NumEnd) {
                        const auto continuation_start = this->_offset;

                        // if (!this->scan_until_get_numeric()) {
                        return ScanResult::Incomplete;
                        // }

                        // Only emit a continuation token if this chunk
                        // actually consumed source characters.
                        if (this->_offset > continuation_start) {
                            add_token(ZLexerTokenKind::Numeric,
                                      continuation_start, this->_offset - 1);
                        }
                    }
            }
        }

        while (!this->eob()) {
            const auto skip_result = this->skip_trivial();

            switch (skip_result) {
                case ZLexerSkipResult::Incomplete:
                    return ScanResult::Incomplete;

                case ZLexerSkipResult::EndOfInput:
                    return ScanResult::EndOfInput;

                case ZLexerSkipResult::NonTrivial:
                    break;
            }

            const auto* p = this->peek();
            if (nullptr == p) ZABAN_UNLIKELY {
                    return ScanResult::EndOfInput;
                }

            const auto start = this->_offset;
            const auto p0    = *p;
            const auto p1    = this->peek(1) != nullptr ? *this->peek(1) : 0;

            if (p0 == '\'' || p0 == '"') {
                if (!scan_string(p0, start)) {
                    return ScanResult::Incomplete;
                }
                continue;
            }

            if (Zaban::Lex::CharUtil::is_digit(p0)) {
                if (!scan_number(start)) {
                    return ScanResult::Incomplete;
                }

                continue;
            }

            if (is_identifier_start(p0)) {
                if (!scan_identifier(start)) {
                    return ScanResult::Incomplete;
                }

                continue;
            }
            // A leading-dot floating point literal, e.g. .10.
            if (p0 == '.' && p1 != 0 && Zaban::Lex::CharUtil::is_digit(p1)) {
                this->advance();
                this->_state = ZLexerInternalState::FloatNumber;

                // if (!this->scan_until_get_numeric()) {
                return ScanResult::Incomplete;
                // }

                add_token(ZLexerTokenKind::Numeric, start, this->_offset - 1);
                continue;
            }

            const auto kind = [p0]() -> std::optional<ZLexerTokenKind> {
                switch (p0) {
                    case '(':
                        return ZLexerTokenKind::LParen;
                    case ')':
                        return ZLexerTokenKind::RParen;
                    case '[':
                        return ZLexerTokenKind::LBrak;
                    case ']':
                        return ZLexerTokenKind::RBrak;
                    case '{':
                        return ZLexerTokenKind::LBrace;
                    case '}':
                        return ZLexerTokenKind::RBrace;
                    case ',':
                        return ZLexerTokenKind::Comma;
                    case ':':
                        return ZLexerTokenKind::Colon;
                    case ';':
                        return ZLexerTokenKind::Semicolon;
                    case '^':
                        return ZLexerTokenKind::Caret;
                    case '~':
                        return ZLexerTokenKind::Tilde;
                    case '.':
                        return ZLexerTokenKind::Dot;
                    case '+':
                        return ZLexerTokenKind::Plus;
                    case '-':
                        return ZLexerTokenKind::Minus;
                    case '*':
                        return ZLexerTokenKind::Asterisk;
                    case '/':
                        return ZLexerTokenKind::Slash;
                    case '%':
                        return ZLexerTokenKind::Percent;
                    case '&':
                        return ZLexerTokenKind::Amp;
                    case '|':
                        return ZLexerTokenKind::Pipe;
                    case '=':
                        return ZLexerTokenKind::Equal;
                    case '!':
                        return ZLexerTokenKind::Exclam;
                    case '?':
                        return ZLexerTokenKind::Qmark;
                    case '<':
                        return ZLexerTokenKind::Lesser;
                    case '>':
                        return ZLexerTokenKind::Greater;
                    case '@':
                        return ZLexerTokenKind::AtSign;
                    default:
                        return std::nullopt;
                }
            }();

            if (kind.has_value()) {
                add_token(*kind, start, start);
            }

            this->advance();
        }

        return ScanResult::Scanned;
    }

    bool ZLexer::scan() {
        return false;
    }
};  // namespace Z::Zaban::Langs::ZLang
