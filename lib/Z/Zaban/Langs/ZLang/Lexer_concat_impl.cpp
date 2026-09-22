#include <Z/Zaban/Langs/ZLang/Lexer.hpp>
#include <Z/Zirsakht/Log/DefaultLogger.hpp>

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

            copy._dc.clear_diagnostics();

            copy._state = this->_state;

            // Keep the original token start from the previous lexer
            copy._token_start = this->_token_start;

            // buffers are contiguous maning this one begins exactly where
            // scanning stopped and that is where it resumes
            copy._start_offset = this->_offset;
            copy._offset       = copy._start_offset;

            copy.scan();
        }

        // Merge tokens at the lexer boundary.
        this->merge(copy);

        this->_state = copy._state;

        this->_tokens.reserve(this->_tokens.size() + copy._tokens.size());

        this->_tokens.insert(this->_tokens.end(), copy._tokens.begin(),
                             copy._tokens.end());

        this->set_offset(copy._offset);

        this->_dc.merge_from(copy._dc);
        this->diagnostics().record_concatenation();

#if ZABAN_DEBUG_MODE
        Zirsakht::Log::DefaultLogger::set_default_log_level(
            Zirsakht::Log::Level::Debug);
        Zirsakht::Log::DefaultLogger::out(Zirsakht::Log::Level::Debug,
                                          "Concat copy object {} >> {}\n",
                                          copy.get_ptr(), this->get_ptr());
#endif
    }

    void ZLexer::concat(ZLexer&& rhs) {
        if (this == &rhs) {
            return;
        }

        if (this->_state != ZLexerInternalState::Normal) {
            // The previous lexer ended in the middle of a token.
            rhs._tokens.clear();

            // scan of this chunk is being thrown away so its
            // diagnostics should be cleared too
            rhs._dc.clear_diagnostics();

            rhs._state = this->_state;

            // Preserve where the unfinished token actually started.
            rhs._token_start  = this->_token_start;
            rhs._start_offset = this->_offset;
            rhs._offset       = rhs._start_offset;

            rhs.scan();
        }

        // Merge boundary tokens before moving the remaining tokens.
        this->merge(rhs);

        this->_state = rhs._state;

        this->_tokens.reserve(this->_tokens.size() + rhs._tokens.size());

        std::ranges::move(rhs._tokens, std::back_inserter(this->_tokens));

        this->set_offset(rhs._offset);

        this->_dc.merge_from(rhs._dc);
        this->diagnostics().record_concatenation();

#if ZABAN_DEBUG_MODE
        Zirsakht::Log::DefaultLogger::set_default_log_level(
            Zirsakht::Log::Level::Debug);
        Zirsakht::Log::DefaultLogger::out(Zirsakht::Log::Level::Debug,
                                          "Concat object {} >> {}\n",
                                          rhs.get_ptr(), this->get_ptr());
#endif
    }
}  // namespace Z::Zaban::Langs::ZLang
