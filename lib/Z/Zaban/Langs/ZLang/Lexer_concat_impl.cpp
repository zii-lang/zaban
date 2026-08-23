#include <Z/Zaban/Langs/ZLang/Lexer.hpp>

namespace Z::Zaban::Langs::ZLang {
    void ZLexer::concat(const ZLexer& rhs) {
        this->validate(ZLexerInvalidationFlag::NoScan);

        ZLexer copy = rhs;
        if (this->_state != ZLexerInternalState::Normal ||
            copy._start_offset != _offset) {
            copy.invalidate(ZLexerInvalidationFlag::NoScan);
        }

        if (copy.has_flag(ZLexerInvalidationFlag::NoScan)) {
            copy._state        = this->_state;
            copy._offset       = this->_offset;
            copy._start_offset = this->_offset;

            copy.validate(ZLexerInvalidationFlag::NoScan);
        }
        this->_state = copy._state;

        _tokens.reserve(_tokens.size() + copy._tokens.size());
        _tokens.insert(_tokens.end(), copy._tokens.begin(), copy._tokens.end());
        this->merge_impl(copy);
    }

    void ZLexer::concat(ZLexer&& rhs) {
        if (this == &rhs) {
            return;
        }

        this->validate(ZLexerInvalidationFlag::NoScan);

        if (this->_state != ZLexerInternalState::Normal ||
            rhs._start_offset != _offset) {
            rhs.invalidate(ZLexerInvalidationFlag::NoScan);
        }

        if (rhs.has_flag(ZLexerInvalidationFlag::NoScan)) {
            rhs._state        = this->_state;
            rhs._offset       = this->_offset;
            rhs._start_offset = this->_offset;

            rhs.validate(ZLexerInvalidationFlag::NoScan);
        }
        this->_state = rhs._state;
        _tokens.reserve(_tokens.size() + rhs._tokens.size());
        _tokens.insert(_tokens.end(),
                       std::make_move_iterator(rhs._tokens.begin()),
                       std::make_move_iterator(rhs._tokens.end()));
        this->merge_impl(rhs);
    }
}  // namespace Z::Zaban::Langs::ZLang
