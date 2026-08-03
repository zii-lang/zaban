#include <Z/Zaban/Config.hpp>
#include <Z/Zaban/Langs/ZLang/Lexer.hpp>
#include <Z/Zaban/Lex/CharUtil.hpp>
#include <Z/Zaban/Lex/ScanUtil.hpp>
#include <iostream>  // TODO: remove this.
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

    ZLexerPositionType ZLexer::get_offset() {
        return this->_offset;
    }

    void ZLexer::set_offset(ZLexerPositionType offset) {
        this->_offset = offset;
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
        if (0 == offset || (this->_buffer_it + offset > this->_buffer.end())) {
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

    bool ZLexer::scan_comment() {
        ZLexerBufferType::const_pointer p0 = this->peek();
        if (nullptr == p0 || '/' != *p0) {
            return false;
        }

        ZLexerBufferType::const_pointer p1 = this->peek(1);
        if (nullptr == p1 ||
            !Zaban::Lex::ScanUtil::is_either_slash_comment(*p0, *p1)) {
            return false;
        }

        this->advance(2);

        if (Zaban::Lex::ScanUtil::is_double_slash_comment(*p0, *p1)) {
            this->set_lexer_state(ZLexerInternalState::LineComment);
            return this->scan_double_slash_close_comment();
        } else {
            this->set_lexer_state(ZLexerInternalState::BlockComment);
            return this->scan_until_block_slash_close_comment();
        }
    }

    bool ZLexer::scan_double_slash_close_comment() {
        if (this->scan_until_newline()) {
            this->set_lexer_state(ZLexerInternalState::Normal);
            return true;
        }
        return false;
    }

    bool ZLexer::scan_until_block_slash_close_comment() {
        for (; this->_buffer_it != this->_buffer.end(); this->advance()) {
            ZLexerBufferType::const_pointer p0 = this->peek();
            ZLexerBufferType::const_pointer p1 = this->peek(1);

            if (nullptr == p0 || nullptr == p1) {
                return false;
            }

            if (Zaban::Lex::ScanUtil::is_block_slash_comment_end(*p0, *p1)) {
                this->advance(2);
                this->set_lexer_state(ZLexerInternalState::Normal);
                return true;
            }
        }

        return false;
    }

    void ZLexer::skip_trivial() {
        ZLexerBufferType::const_pointer p  = nullptr;
        ZLexerBufferType::value_type    p0 = 0;
        ZLexerBufferType::value_type    p1 = 0;

        for (; this->_buffer_it != this->_buffer.end();) {
            // Reset to normal state.
            this->_state = ZLexerInternalState::Normal;

            p0 = (p = this->peek()) == nullptr ? 0 : *p;

            /* this is unlieky because we already check if iterator has not
            encountered end of line. */
            if (0 == p0) Z_UNLIKELY {
                    return;
                }

            if (Zaban::Lex::CharUtil::is_whitespace(p0)) {
                this->set_lexer_state(ZLexerInternalState::Whitespace);
                this->advance();
                continue;
            }

            bool scan_comment_result = this->scan_comment();
            if (!scan_comment_result &&
                this->_state != ZLexerInternalState::Normal)
                Z_UNLIKELY {
                    this->_error = ZLexerError::UnterminatedComment;
                    return;
                }
            else {
                break;
            }
        }
    }

#define ZADD_TOKEN(kind, begin, end) \
    this->_tokens.emplace_back(      \
        kind, SourcePositionRange<ZLexerPositionType>(begin, end))

    bool ZLexer::scan() {
        if (this->_buffer_it == this->_buffer.end()) {
            return false;
        }

        if (ZLexerInternalState::Normal != this->_state) {
            if (ZLexerInternalState::LineComment == this->_state) {
                if (!this->scan_double_slash_close_comment()) {
                    return false;
                }
            } else if (ZLexerInternalState::BlockComment == this->_state) {
                if (!this->scan_until_block_slash_close_comment()) {
                    return false;
                }
            } else if (ZLexerInternalState::Whitespace == this->_state) {
                this->_state = ZLexerInternalState::Normal;
            }
        }

        ZLexerBufferType::value_type p0 = 0;
        ZLexerBufferType::value_type p1 = 0;

        for (; this->_buffer_it != this->_buffer.end();) {
            this->skip_trivial();

            ZLexerBufferType::const_pointer p = this->peek();
            if (nullptr == p) Z_UNLIKELY {
                    return false;
                }

            p0 = *p;
            p1 = (p = this->peek(1)) == nullptr ? 0 : *p;

            switch (p0) {
                case '(':
                    ZADD_TOKEN(ZLexerTokenKind::LParen, this->_offset,
                               this->_offset + 1);
                    this->advance();
                    break;
                case ')':
                    ZADD_TOKEN(ZLexerTokenKind::RParen, this->_offset,
                               this->_offset + 1);
                    this->advance();
                    break;
                default:
                    this->advance();
                    break;
            }
        }
        return true;
    }

    std::vector<ZLexerTokenType> ZLexer::finalize() {
        return this->_tokens;
    }

    LexerDiagnostics ZLexer::diagnostics() {
        return LexerDiagnostics();
    }
}  // namespace Z::Zaban::Langs::ZLang
