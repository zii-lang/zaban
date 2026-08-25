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
        if (this->eob()) {
            return ScanResult::EndOfInput;
        }
        this->_dc.record_scan();

        if (this->_state != ZLexerInternalState::Normal) {
            ScanResult fix_result = this->scan_fix(0);  // TODO: fix this
            if (ScanResult::Scanned != fix_result) {
                return fix_result;
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

            const auto start = this->get_offset();
            const auto p0    = *p;
            const auto p1    = this->peek(1) != nullptr ? *this->peek(1) : 0;

            if (p0 == '\'' || p0 == '"') {
                if (p0 == '\'') {
                    this->set_state(ZLexerInternalState::SQString);
                } else {
                    this->set_state(ZLexerInternalState::DQString);
                }
                ScanResult fix_result = this->scan_fix(start);
                if (ScanResult::Scanned != fix_result) {
                    return fix_result;
                }
                continue;
            }

            if (Zaban::Lex::CharUtil::is_digit(p0)) {
                if ('0' == p0) {
                    this->set_state(ZLexerInternalState::ZeroStart);
                } else {
                    this->set_state(ZLexerInternalState::Number);
                }
                this->advance();
                ScanResult fix_result = this->scan_fix(start);
                if (ScanResult::Scanned != fix_result) {
                    return fix_result;
                }

                continue;
            }

            if (is_identifier_start(p0)) {
                this->set_state(ZLexerInternalState::Identifier);

                ScanResult fix_result = this->scan_fix(start);
                if (ScanResult::Scanned != fix_result) {
                    return fix_result;
                }

                continue;
            }

            // A leading-dot floating point literal, e.g. .10.
            if (p0 == '.' && p1 != 0 && Zaban::Lex::CharUtil::is_digit(p1)) {
                this->advance();
                this->set_state(ZLexerInternalState::FloatNumber);

                ScanResult fix_result = this->scan_fix(start);
                if (ScanResult::Scanned != fix_result) {
                    return fix_result;
                }

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
                add_token(*this, *kind, start, start);
            }

            this->advance();
        }

        this->_flags = unset(this->_flags, ZLexerInvalidationFlag::NeedsScan);
        return ScanResult::Scanned;
    }

    bool ZLexer::scan() {
        ScanResult res = this->scan_impl();
        if (ScanResult::Scanned == res) {
            return true;
        }
        return false;
    }
};  // namespace Z::Zaban::Langs::ZLang
