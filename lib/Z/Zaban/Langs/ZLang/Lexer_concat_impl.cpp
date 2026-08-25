#include <Z/Zaban/Langs/ZLang/Lexer.hpp>

#if ZABAN_DEBUG_MODE
# include <iostream>
#endif

namespace Z::Zaban::Langs::ZLang {
    void ZLexer::concat(const ZLexer& rhs) {
        ZLexer copy = rhs;

        const bool needs_scan = this->_state != ZLexerInternalState::Normal ||
                                copy._start_offset != this->_offset;

        if (needs_scan) {
            // The previous lexer ended in the middle of a token.
            // Continue scanning using the new buffer.
            copy._tokens.clear();
            copy._state        = this->_state;
            copy._offset       = this->_offset;
            copy._start_offset = this->_offset;

            const auto result = copy.scan();

            if (!result) {
                // TODO: merge diagnostics / propagate error.
            }
        }

        this->_state = copy._state;

        this->_tokens.reserve(this->_tokens.size() + copy._tokens.size());

        this->_tokens.insert(this->_tokens.end(), copy._tokens.begin(),
                             copy._tokens.end());

        this->merge();
        this->diagnostics().record_concatenation();
        this->_dc.set_scan_count(this->diagnostics().scan_count() +
                                 copy.diagnostics().scan_count());

#if ZABAN_DEBUG_MODE
        std::cout << "concat obj: " << copy.get_ptr() << " >> "
                  << this->get_ptr() << std::endl;
#endif
    }

    void ZLexer::concat(ZLexer&& rhs) {
        if (this == &rhs) {
            return;
        }

        const bool needs_scan = this->_state != ZLexerInternalState::Normal ||
                                rhs._start_offset != this->_offset;

        if (needs_scan) {
            rhs._tokens.clear();
            rhs._state        = this->_state;
            rhs._offset       = this->_offset;
            rhs._start_offset = this->_offset;

            const auto result = rhs.scan();

            if (!result) {
                // TODO: merge diagnostics / propagate error.
            }
        }

        this->_state = rhs._state;

        this->_tokens.reserve(this->_tokens.size() + rhs._tokens.size());

        std::ranges::move(rhs._tokens, std::back_inserter(this->_tokens));

        this->merge();
        this->diagnostics().record_concatenation();
        this->_dc.set_scan_count(this->diagnostics().scan_count() +
                                 rhs.diagnostics().scan_count());

#if ZABAN_DEBUG_MODE
        std::cout << "concat obj: " << rhs.get_ptr() << " >> "
                  << this->get_ptr() << std::endl;
#endif
    }
}  // namespace Z::Zaban::Langs::ZLang
