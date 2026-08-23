#include <Z/Zaban/Langs/ZLang/Lexer.hpp>

namespace Z::Zaban::Langs::ZLang {
    ZLexerBufferType::const_pointer ZLexer::peek() const {
        return this->peek(0);
    }

    ZLexerBufferType::const_pointer ZLexer::peek(
        const ZLexerPositionType distance) const {
        const auto index = this->_offset - this->_start_offset;
        if (index + distance >= this->_buffer.size()) {
            return nullptr;
        }

        return this->_buffer.data() + index + distance;
    }

    void ZLexer::advance() {
        this->advance(1);
    }

    void ZLexer::advance() {
        if (this->_offset < this->_start_offset + this->_buffer.size()) {
            ++this->_offset;
        }
    }

    void ZLexer::advance(const ZLexerPositionType distance) {
        const auto end = this->_start_offset + this->_buffer.size();

        if (this->_offset + distance >= end) {
            this->_offset = end;
            return;
        }

        this->_offset += distance;
    }
}  // namespace Z::Zaban::Langs::ZLang
