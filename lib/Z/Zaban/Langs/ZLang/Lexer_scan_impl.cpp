// System imports
#include <unordered_map>
// Z imports
#include <Z/Zaban/Langs/ZLang/Lexer.hpp>
#include <Z/Zaban/Lex/ScanUtil.hpp>

namespace Z::Zaban::Langs::ZLang {
    static const std::unordered_map<std::string, ZLexerTokenKind>
        ZLangKeywords = {
            {"null", ZLexerTokenKind::Null},
            {"true", ZLexerTokenKind::True},
            {"false", ZLexerTokenKind::False},
            {"let", ZLexerTokenKind::Let},
            {"type", ZLexerTokenKind::Type},
            {"return", ZLexerTokenKind::Return},
            {"struct", ZLexerTokenKind::Struct},
            {"enum", ZLexerTokenKind::Enum},
            {"if", ZLexerTokenKind::If},
            {"endif", ZLexerTokenKind::EndIf},
            {"loop", ZLexerTokenKind::Loop},
            {"endloop", ZLexerTokenKind::EndLoop},
            {"func", ZLexerTokenKind::Func},
            {"vari", ZLexerTokenKind::Vari},
            {"break", ZLexerTokenKind::Break},
            {"continue", ZLexerTokenKind::Continue},
            {"goto", ZLexerTokenKind::Goto},
            {"label", ZLexerTokenKind::Label},
    };

    static bool is_identifier_start(const char ch) noexcept {
        return ch == '_' || Lex::CharUtil::is_alpha(ch);
    }

    static bool is_identifier_continue(const char ch) noexcept {
        return is_identifier_start(ch) || Lex::CharUtil::is_digit(ch);
    }

    static ZLexerTokenKind classify_identifier(const std::string& text) {
        const auto it = ZLangKeywords.find(text);

        if (it != ZLangKeywords.end()) {
            return it->second;
        }

        return ZLexerTokenKind::Identifier;
    }

    static std::string token_text(const ZLexer&          lexer,
                                  const ZLexerTokenType& token) {
        const auto buffer = lexer.get_buffer();

        const auto begin = static_cast<std::size_t>(token.range.begin -
                                                    lexer.get_start_offset());

        const auto end = static_cast<std::size_t>(token.range.end -
                                                  lexer.get_start_offset() + 1);

        if (begin > buffer.size() || end > buffer.size() || begin > end) {
            return {};
        }

        return std::string(buffer.data() + begin, end - begin);
    }

    ScanResult ZLexer::scan_impl() {
        if (this->_buffer_it == this->_buffer.end()) {
            return ScanResult::EndOfInput;
        }

        auto add_token = [this](ZLexerTokenKind kind, ZLexerPositionType start,
                                ZLexerPositionType end) {
            this->_tokens.emplace_back(
                kind, OffsetRange<ZLexerPositionType>(start, end));
        };

        if (this->_state != ZLexerInternalState::Normal) {
            switch (this->_state) {
                case ZLexerInternalState::LineComment:
                    if (!this->scan_double_slash_close_comment()) {
                        return ScanResult::Incomplete;
                    }
                    break;

                case ZLexerInternalState::BlockComment:
                    if (!this->scan_until_block_slash_close_comment()) {
                        return ScanResult::Incomplete;
                    }
                    break;

                case ZLexerInternalState::SQString:
                case ZLexerInternalState::DQString:
                    if (!this->scan_until_eos()) {
                        return ScanResult::Incomplete;
                    }
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

                    // The next chunk did not actually continue the identifier.
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
            }
        }
    }
};  // namespace Z::Zaban::Langs::ZLang
