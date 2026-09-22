#include <Z/Zaban/Langs/ZLang/Lexer.hpp>

#include "Z/Zaban/Langs/ZLang/LexerDiagnostic.hpp"
#include "Z/Zaban/Langs/ZLang/TokenKind.hpp"

namespace Z::Zaban::Langs::ZLang {
    ZLexer::ZLexer(ZLexerBufferType& buffer) :
        Zaban::Lex::Lexer<ZLexerTokenType, ZLexerPositionType,
                          ZLexerBufferType>(buffer) {
        this->_pending = TokenFlags::AtLineStart;
    };

    ZLexer::ZLexer(ZLexerBufferType& buffer, ZLexerPositionType start_pos) :
        Zaban::Lex::Lexer<ZLexerTokenType, ZLexerPositionType,
                          ZLexerBufferType>(buffer, start_pos) {
        this->_pending =
            (start_pos == 0) ? TokenFlags::AtLineStart : TokenFlags::None;
    };

    ZLexerInternalState ZLexer::get_state() const noexcept {
        return this->_state;
    }

    void ZLexer::set_state(ZLexerInternalState state) {
        this->_state = state;
    }

    ZLexerBufferType ZLexer::get_buffer() const noexcept {
        return this->_buffer;
    }

    void ZLexer::set_buffer(ZLexerBufferType& buffer) {
        this->_buffer = buffer;
    }

    ZLexerPositionType ZLexer::get_offset() {
        return this->_offset;
    }

    void ZLexer::set_offset(ZLexerPositionType offset) {
        this->_offset = offset;
    }

    ZLexerPositionType ZLexer::get_start_offset() const noexcept {
        return this->_start_offset;
    }
    ZLexerPositionType ZLexer::get_token_start() const noexcept {
        return this->_token_start;
    }
    void ZLexer::mark_token_start(ZLexerPositionType offset) {
        this->_token_start = offset;
    }

    ZLexerPositionType ZLexer::get_end_offset() const noexcept {
        return this->_start_offset + this->_buffer.size();
    }

    std::vector<ZLexerTokenType>& ZLexer::get_tokens() {
        return this->_tokens;
    }

    void ZLexer::mark_pending(TokenFlags p) {
        this->_pending |= p;
    }

    TokenFlags ZLexer::take_pending() {
        auto p         = this->_pending;
        this->_pending = TokenFlags::None;
        return p;
    }

    void ZLexer::set_tokens(std::vector<ZLexerTokenType> tokens) {
        this->_tokens = std::move(tokens);
    }

    ZLexerTokenType& ZLexer::get_token(std::size_t index) {
        return this->_tokens[index];
    }

    bool ZLexer::eob() const {
        return this->_offset >= this->_start_offset + this->_buffer.size();
    }

    void ZLexer::report(ZLexerDiagnosticKind            kind,
                        OffsetRange<ZLexerPositionType> range,
                        std::string_view                reason) {
        this->_dc.add(ZLexerDiagnostic(kind, reason, range));
    }

    void ZLexer::close_open_construct() {
        const OffsetRange<ZLexerPositionType> range(this->_token_start,
                                                    this->_offset);
        switch (this->_state) {
            case ZLexerInternalState::SQString:
            case ZLexerInternalState::DQString: {
                // continue string function emits nothing when the quote never
                // closes. we need to emit the chunk anyway so the text is not
                // lost and the editor can still see a str where the src has one
                add_token(*this, TokenKind::String, this->_token_start,
                          this->_offset);
                this->report(ZLexerDiagnosticKind::ErrorUnterminatedString,
                             range, "String literal is not terminated");
                break;
            }
            case ZLexerInternalState::BlockComment: {
                this->report(ZLexerDiagnosticKind::ErrorUnterminatedComment,
                             range, "Block comment is not terminated");
                break;
            }
            default:
                // normal and linecomment both close at EOI and continue_number
                // already reports every incomplete numeric literal
                break;
        }
        this->_state = ZLexerInternalState::Normal;
    }

    std::vector<ZLexerTokenType> ZLexer::finalize() {
        if (has(this->_flags, ZLexerInvalidationFlag::NeedsScan)) {
            this->scan();
        }
        if (has(this->_flags, ZLexerInvalidationFlag::NeedsMerge)) {
            this->merge();
        }
        this->close_open_construct();

        add_token(*this, TokenKind::Eof, this->_offset, this->_offset);
        return this->_tokens;
    }

    Lex::LexerDiagnosticContextBase& ZLexer::diagnostics() {
        return this->_dc;
    }

    ZLexer& ZLexer::operator<<(const ZLexer& rhs) {
        this->concat(rhs);
        return *this;
    }

    ZLexer& ZLexer::operator<<(ZLexer&& rhs) {
        concat(std::move(rhs));
        return *this;
    }
}  // namespace Z::Zaban::Langs::ZLang
