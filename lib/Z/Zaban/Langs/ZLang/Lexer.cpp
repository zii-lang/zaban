#include <Z/Zaban/Langs/ZLang/Lexer.hpp>

namespace Z::Zaban::Langs::ZLang {
    ZLexer::ZLexer(ZLexerBufferType& buffer) :
        Zaban::Lex::Lexer<ZLexerTokenType, ZLexerPositionType,
                          ZLexerBufferType>(buffer) {};

    ZLexer::ZLexer(ZLexerBufferType& buffer, ZLexerPositionType start_pos) :
        Zaban::Lex::Lexer<ZLexerTokenType, ZLexerPositionType,
                          ZLexerBufferType>(buffer, start_pos) {};

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

    ZLexerPositionType ZLexer::get_end_offset() const noexcept {
        return (this->_start_offset == 0 ? 1 : this->_start_offset) +
               this->_buffer.size();
    }

    std::vector<ZLexerTokenType>& ZLexer::get_tokens() {
        return this->_tokens;
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

    std::vector<ZLexerTokenType> ZLexer::finalize() {
        if (has(this->_flags, ZLexerInvalidationFlag::NeedsScan)) {
            this->scan();
        }
        if (has(this->_flags, ZLexerInvalidationFlag::NeedsMerge)) {
            this->merge();
        }
        if (static_cast<std::uint8_t>(this->_flags) != 0) {
            // TODO: report error.
        }
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
