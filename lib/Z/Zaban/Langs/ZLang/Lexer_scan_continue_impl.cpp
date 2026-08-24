// System imports
#include <unordered_map>
// Z imports
#include <Z/Zaban/Langs/ZLang/Lexer.hpp>
#include <Z/Zaban/Lex/ScanUtil.hpp>

namespace Z::Zaban::Langs::ZLang {
    static ScanResult continue_identifier(ZLexer& lexer) {
        const auto start = lexer.get_offset();

        while (const auto* p = lexer.peek()) {
            if (!is_identifier_continue(*p)) {
                break;
            }

            lexer.advance();
        }

        if (lexer.get_offset() == start) {
            lexer.set_state(ZLexerInternalState::Normal);
            return ScanResult::Scanned;
        }

        const auto text = std::string(lexer.get_buffer().substr(
            start - lexer.get_start_offset(), lexer.get_offset() - start));

        add_token(lexer, classify_identifier(text), start,
                  lexer.get_offset() - 1);

        lexer.set_state(ZLexerInternalState::Normal);
        return ScanResult::Scanned;
    }

    static ScanResult continue_line_comment(ZLexer& lexer) {
        const auto start = lexer.get_offset();

        while (const auto* p = lexer.peek()) {
            const auto* next = lexer.peek(1);

            std::size_t newline_count;
            if (Lex::ScanUtil::is_newline_seq(*p, next ? *next : 0,
                                              &newline_count)) {
                lexer.advance(newline_count);
                break;
            }

            lexer.advance();
        }

        if (lexer.get_offset() == start) {
            // Nothing continued the line comment.
            lexer.set_state(ZLexerInternalState::Normal);
            return ScanResult::Scanned;
        }

        lexer.set_state(ZLexerInternalState::Normal);
        return ScanResult::Scanned;
    }

    static ScanResult continue_block_comment(ZLexer& lexer) {
        const auto start = lexer.get_offset();

        while (const auto* p = lexer.peek()) {
            const auto* next = lexer.peek(1);

            if (*p == '*' && next != nullptr && *next == '/') {
                lexer.advance(2);
                lexer.set_state(ZLexerInternalState::Normal);
                return ScanResult::Scanned;
            }

            lexer.advance();
        }

        if (lexer.get_offset() == start) {
            // Nothing continued the block comment.
            lexer.set_state(ZLexerInternalState::Normal);
            return ScanResult::Scanned;
        }

        lexer.set_state(ZLexerInternalState::BlockComment);
        return ScanResult::EndOfInput;
    }

    static ScanResult continue_string(ZLexer& lexer) {
        const auto start = lexer.get_offset();

        const char quote =
            lexer.get_state() == ZLexerInternalState::SQString ? '\'' : '"';

        bool escaped = false;

        while (const auto* p = lexer.peek()) {
            if (escaped) {
                escaped = false;
                lexer.advance();
                continue;
            }

            if (*p == '\\') {
                escaped = true;
                lexer.advance();
                continue;
            }

            if (*p == quote) {
                lexer.advance();
                lexer.set_state(ZLexerInternalState::Normal);
                return ScanResult::Scanned;
            }

            lexer.advance();
        }

        if (lexer.get_offset() == start) {
            // Nothing continued the string.
            lexer.set_state(ZLexerInternalState::Normal);
            return ScanResult::Scanned;
        }

        lexer.set_state(quote == '\'' ? ZLexerInternalState::SQString
                                      : ZLexerInternalState::DQString);

        return ScanResult::EndOfInput;
    }

    static ScanResult continue_number(ZLexer& lexer) {
        const auto start = lexer.get_offset();

        while (const auto* p = lexer.peek()) {
            const auto state = lexer.get_state();

            switch (state) {
                case ZLexerInternalState::STATE_NumStart:
                    if (Lex::CharUtil::is_digit(*p)) {
                        lexer.set_state(ZLexerInternalState::Number);
                        lexer.advance();
                        continue;
                    }

                    if (*p == '.') {
                        lexer.set_state(ZLexerInternalState::FloatNumber);
                        lexer.advance();
                        continue;
                    }

                    break;

                case ZLexerInternalState::ZeroStart:
                    if (*p == 'x' || *p == 'X') {
                        lexer.set_state(ZLexerInternalState::HexNumber);
                        lexer.advance();
                        continue;
                    }

                    if (*p == 'o' || *p == 'O') {
                        lexer.set_state(ZLexerInternalState::OctNumber);
                        lexer.advance();
                        continue;
                    }

                    if (*p == 'b' || *p == 'B') {
                        lexer.set_state(ZLexerInternalState::BinNumber);
                        lexer.advance();
                        continue;
                    }

                    if (Lex::CharUtil::is_digit(*p)) {
                        lexer.set_state(ZLexerInternalState::Number);
                        lexer.advance();
                        continue;
                    }

                    if (*p == '.') {
                        lexer.set_state(ZLexerInternalState::FloatNumber);
                        lexer.advance();
                        continue;
                    }

                    break;

                case ZLexerInternalState::HexNumber:
                    if (Lex::CharUtil::is_hex_digit(*p)) {
                        lexer.advance();
                        continue;
                    }
                    break;

                case ZLexerInternalState::OctNumber:
                    if (Lex::CharUtil::is_oct_digit(*p)) {
                        lexer.advance();
                        continue;
                    }
                    break;

                case ZLexerInternalState::BinNumber:
                    if (*p == '0' || *p == '1') {
                        lexer.advance();
                        continue;
                    }
                    break;

                case ZLexerInternalState::Number:
                    if (Lex::CharUtil::is_digit(*p)) {
                        lexer.advance();
                        continue;
                    }

                    if (*p == '.') {
                        lexer.set_state(ZLexerInternalState::FloatNumber);
                        lexer.advance();
                        continue;
                    }

                    if (*p == 'e' || *p == 'E') {
                        lexer.set_state(ZLexerInternalState::ScientificNumber);
                        lexer.advance();
                        continue;
                    }

                    break;

                case ZLexerInternalState::FloatNumber:
                    if (Lex::CharUtil::is_digit(*p)) {
                        lexer.advance();
                        continue;
                    }

                    if (*p == 'e' || *p == 'E') {
                        lexer.set_state(ZLexerInternalState::ScientificNumber);
                        lexer.advance();
                        continue;
                    }

                    break;

                case ZLexerInternalState::ScientificNumber:
                    if (Lex::CharUtil::is_digit(*p)) {
                        lexer.advance();
                        continue;
                    }

                    if (*p == '+' || *p == '-') {
                        lexer.advance();
                        continue;
                    }

                    break;

                default:
                    break;
            }

            break;
        }

        if (lexer.get_offset() == start) {
            lexer.set_state(ZLexerInternalState::Normal);
            return ScanResult::Scanned;
        }

        add_token(lexer, ZLexerTokenKind::Numeric, start,
                  lexer.get_offset() - 1);

        lexer.set_state(ZLexerInternalState::Normal);
        return ScanResult::Scanned;
    }

    ScanResult ZLexer::scan_fix() {
        switch (this->get_state()) {
            case ZLexerInternalState::LineComment:
                return continue_line_comment(*this);
            case ZLexerInternalState::BlockComment:
                return continue_block_comment(*this);
            case ZLexerInternalState::SQString:
            case ZLexerInternalState::DQString:
                return continue_string(*this);
            case ZLexerInternalState::Identifier:
                return continue_identifier(*this);
            case ZLexerInternalState::STATE_NumStart:
            case ZLexerInternalState::ZeroStart:
            case ZLexerInternalState::HexNumber:
            case ZLexerInternalState::OctNumber:
            case ZLexerInternalState::BinNumber:
            case ZLexerInternalState::Number:
            case ZLexerInternalState::FloatNumber:
            case ZLexerInternalState::ScientificNumber:
                return continue_number(*this);
            case ZLexerInternalState::Normal:
                return ScanResult::Scanned;
            case ZLexerInternalState::Error:
                return ScanResult::Error;
            default:
                return ScanResult::Error;
        }
    }
}  // namespace Z::Zaban::Langs::ZLang
