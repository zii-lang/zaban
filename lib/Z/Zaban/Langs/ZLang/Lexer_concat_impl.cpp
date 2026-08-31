#include <Z/Zaban/Langs/ZLang/Lexer.hpp>

namespace Z::Zaban::Langs::ZLang {
    void ZLexer::concat(const ZLexer& rhs) {
        if (this == &rhs) {
            return;
        }

        ZLexer copy = rhs;

        if (this->_state != ZLexerInternalState::Normal) {
            // The previous lexer ended in the middle of a token.
            // Continue scanning using the rhs buffer.
            copy._tokens.clear();
            copy._state = this->_state;

            // Continue from the current lexer position.
            copy._offset = this->_offset + 1;

            // IMPORTANT:
            // Keep the original token start from the previous lexer.
            copy._start_offset = this->get_end_offset() + 1;

            const auto result = copy.scan();

            if (!result) {
                // TODO: merge diagnostics / propagate error.
            }
        }

        // Merge tokens at the lexer boundary.
        this->merge(copy);

        this->_state = copy._state;

        this->_tokens.reserve(this->_tokens.size() + copy._tokens.size());

        this->_tokens.insert(this->_tokens.end(), copy._tokens.begin(),
                             copy._tokens.end());

        this->set_offset(copy._offset);

        this->diagnostics().record_concatenation();

        this->_dc.set_scan_count(this->diagnostics().scan_count() +
                                 copy.diagnostics().scan_count());

#if ZABAN_DEBUG_MODE && ZABAN_USE_SPDLOG
        spdlog::debug("Concat copy object {} >> {}", copy.get_ptr(),
                      this->get_ptr());
#endif
    }

    void ZLexer::concat(ZLexer&& rhs) {
        if (this == &rhs) {
            return;
        }

        if (this->_state != ZLexerInternalState::Normal) {
            // The previous lexer ended in the middle of a token.
            rhs._tokens.clear();

            rhs._state = this->_state;

            rhs._offset = this->_offset + 1;

            // Preserve where the unfinished token actually started.
            rhs._start_offset = this->get_end_offset() + 1;

            const auto result = rhs.scan();

            if (!result) {
                // TODO: merge diagnostics / propagate error.
            }
        }

        // Merge boundary tokens before moving the remaining tokens.
        this->merge(rhs);

        this->_state = rhs._state;

        this->_tokens.reserve(this->_tokens.size() + rhs._tokens.size());

        std::ranges::move(rhs._tokens, std::back_inserter(this->_tokens));

        this->set_offset(rhs._offset);

        this->diagnostics().record_concatenation();

        this->_dc.set_scan_count(this->diagnostics().scan_count() +
                                 rhs.diagnostics().scan_count());

#if ZABAN_DEBUG_MODE && ZABAN_USE_SPDLOG
        spdlog::debug("Concat object {} >> {}", rhs.get_ptr(), this->get_ptr());
#endif
    }
}  // namespace Z::Zaban::Langs::ZLang
