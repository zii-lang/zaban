#pragma once

#include <Z/Zaban/Lex/Token.hpp>
#include <vector>

namespace Z::Zaban::Parse {
    template<typename TokenKind, typename OffsetType>
    class TokenStream {
        using TokenType = Lex::Token<TokenKind, OffsetType>;

       private:
        // NOTE: peek(), previous() and advance() hand out pointers into this
        // vector, so it must not be modified after construction.
        std::vector<TokenType> m_tokens;
        // WARNING: this assumes that token count fits in OffsetType
        // we need to change it to std::size_t probably if we ever face an error
        // cause by macro expansion (number of tokens exceed the number of
        // source bytes)
        mutable OffsetType m_offset = 0;

       public:
        TokenStream() = default;
        TokenStream(std::vector<TokenType> tokens) :
            m_tokens(std::move(tokens)) {};

        /** @brief Returns the token n positions ahead of the current one, or
         * nullptr if it is past the end of the stream. */
        const TokenType* peek(std::size_t n = 0) const {
            if (this->m_offset + n >= this->m_tokens.size()) return nullptr;

            return &this->m_tokens[this->m_offset + n];
        }

        /** @brief Returns the most recently consumed token, or nullptr if
         * nothing has been consumed yet. */
        const TokenType* previous() const {
            if (this->m_offset == 0) return nullptr;

            return &this->m_tokens[this->m_offset - 1];
        }

        /** @brief Returns whether the current token is of the given kind. */
        bool check(TokenKind kind) const {
            const auto* token = this->peek();
            return token != nullptr && token->kind == kind;
        }

        /** @brief Consumes the current token if it is of the given kind. */
        bool match(TokenKind kind) const {
            if (!this->check(kind)) return false;

            this->advance();
            return true;
        }

        /** @brief Consumes the current token and returns it, or nullptr if the
         * stream is at its end. */
        const TokenType* advance() const {
            if (this->end()) return nullptr;

            return &this->m_tokens[this->m_offset++];
        }

        /** @brief Returns whether the stream is exhausted or positioned on the
         * Eof token. */
        bool end() const {
            const auto* token = this->peek();
            return token == nullptr || token->kind == TokenKind::Eof;
        }
    };
}  // namespace Z::Zaban::Parse
