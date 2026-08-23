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
            // Nothing continued the identifier.
            lexer.set_state(ZLexerInternalState::Normal);
            return ScanResult::Scanned;
        }

        add_token(lexer, ZLexerTokenKind::Identifier, start,
                  lexer.get_offset() - 1);

        if (lexer.peek() == nullptr) {
            lexer.set_state(ZLexerInternalState::Identifier);
            return ScanResult::EndOfInput;
        }

        lexer.set_state(ZLexerInternalState::Normal);
        return ScanResult::Scanned;
    }

    static ScanResult continue_scan(ZLexer& lexer) {
        switch (lexer.get_state()) {
            case ZLexerInternalState::LineComment:
                return continue_line_comment(lexer);

            case ZLexerInternalState::BlockComment:
                return continue_block_comment(lexer);

            case ZLexerInternalState::SQString:
            case ZLexerInternalState::DQString:
                return continue_string(lexer);

            case ZLexerInternalState::Identifier:
                return continue_identifier(lexer);

            case ZLexerInternalState::STATE_NumStart:
            case ZLexerInternalState::ZeroStart:
            case ZLexerInternalState::HexNumber:
            case ZLexerInternalState::OctNumber:
            case ZLexerInternalState::BinNumber:
            case ZLexerInternalState::Number:
            case ZLexerInternalState::FloatNumber:
            case ZLexerInternalState::ScientificNumber:
                return continue_number(lexer);

            case ZLexerInternalState::Normal:
                return ScanResult::Scanned;

            case ZLexerInternalState::Error:
                return ScanResult::Error;

            default:
                return ScanResult::Error;
        }
    }
}  // namespace Z::Zaban::Langs::ZLang
