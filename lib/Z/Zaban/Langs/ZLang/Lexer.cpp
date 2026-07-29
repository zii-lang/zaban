#include <Z/Zaban/Config.hpp>
#include <Z/Zaban/Langs/ZLang/Lexer.hpp>
#include <Z/Zaban/Lex/CharUtil.hpp>
#include <Z/Zaban/Lex/ScanUtil.hpp>
#include <memory>
#include <unordered_map>

namespace Z::Zaban::Langs::ZLang {
    const static std::unordered_map<std::string, ZLexerTokenKind>
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

    ZLexer::ZLexer(ZLexerBufferType& buffer) :
        Zaban::Lex::Lexer<ZLexerTokenType, ZLexerPositionType,
                          ZLexerBufferType>(buffer),
        _buffer_it(buffer.begin()) {};

    void ZLexer::set_buffer(ZLexerBufferType& buffer) {
        this->_buffer    = buffer;
        this->_buffer_it = this->_buffer.begin();
    }

    bool ZLexer::analyze() {
        return false;
    }

    std::vector<ZLexerTokenType> ZLexer::finalize() {
        return std::vector<ZLexerTokenType>();
    }

    LexerDiagnostics ZLexer::diagnostics() {
        return LexerDiagnostics();
    }

    ZLexerBufferType::const_pointer ZLexer::peek() const {
        return this->peek(0);
    }

    ZLexerBufferType::const_pointer ZLexer::peek(
        const ZLexerPositionType offset) const {
        if (this->_buffer_it + offset >= this->_buffer.end()) {
            return nullptr;
        }
        return std::to_address(this->_buffer_it + offset);
    }

    void ZLexer::advance() {
        this->advance(1);
    }

    void ZLexer::advance(const ZLexerPositionType offset) {
        if (0 == offset || (this->_buffer_it + offset >= this->_buffer.end())) {
            return;
        }
        this->_buffer_it += offset;
        this->_offset += offset;
    }

    void ZLexer::set_lexer_state(const ZLexerInternalState state) {
        this->_state = state;
    }

    bool ZLexer::scan_newline() {
        ZLexerBufferType::const_pointer p0 = this->peek();
        if (nullptr == p0 || !Lex::CharUtil::is_linefeed(*p0)) {
            return false;
        }

        // true from here
        ZLexerBufferType::const_pointer p1 = this->peek(1);
        if (nullptr == p1) {
            this->advance();
            return true;
        }

        ZLexerPositionType line_char_count = 0;
        Lex::ScanUtil::is_newline_seq(*p0, *p1, &line_char_count);
        this->advance(line_char_count);
        return true;
    }

    bool ZLexer::scan_until_newline() {
        for (; this->_buffer_it != this->_buffer.end(); this->advance()) {
            if (this->scan_newline()) {
                return true;
            }
        }
        return false;
    }

    bool ZLexer::scan_double_slash_comment() {
        if (this->scan_until_newline()) {
            this->set_lexer_state(ZLexerInternalState::Normal);
            return true;
        }
        return false;
    }

    bool ZLexer::scan_until_block_slash_comment() {
        for (; this->_buffer_it != this->_buffer.end(); this->advance()) {
        }
    }

    void ZLexer::skip_trivial() {
        ZLexerBufferType::const_pointer p  = nullptr;
        ZLexerBufferType::value_type    p0 = 0;
        ZLexerBufferType::value_type    p1 = 0;

        for (; this->_buffer_it != this->_buffer.end(); this->advance()) {
            p0 = (p = this->peek()) == nullptr ? 0 : *p;

            /* this is unlieky because we already check if iterator has not
            encountered end of line. */
            if (0 == p0) Z_UNLIKELY {
                    return;
                }

            if (Zaban::Lex::CharUtil::is_whitespace(p0)) {
                continue;
            }

            p1 = (p = this->peek(1)) == nullptr ? 0 : *p;

            if (0 == p1) {
                return;
            }

            if (!Zaban::Lex::ScanUtil::is_either_slash_comment(p0, p1)) {
                break;
            }

            // We got either comement starters so we move forward.
            this->advance(2);

            if (Zaban::Lex::ScanUtil::is_double_slash_comment(p0, p1)) {
                this->set_lexer_state(ZLexerInternalState::LineComment);
                return;
            } else {
                this->set_lexer_state(ZLexerInternalState::BlockComment);
                return;
            }
        }
    }

    bool ZLexer::analyze() {
        for (; this->_buffer_it != this->_buffer.end(); this->advance()) {
        }
    }
}  // namespace Z::Zaban::Langs::ZLang
