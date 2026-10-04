#pragma once

#include <Z/Zaban/Lex/Token.hpp>
#include <vector>

namespace Z::Zaban::Parse {
    template<typename TokenKind, typename OffsetType>
    class TokenStream {
        using TokenType   = Lex::Token<TokenKind, OffsetType>;
        using SPTokenType = std::shared_ptr<TokenType>;

       private:
        std::vector<TokenType> m_tokens;
        // WARNING: this assumes that token count fits in OffsetType
        // we need to change it to std::size_t probably if we ever face an error
        // cause by macro expansion (number of tokens exceed the number of
        // source bytes)
        mutable OffsetType m_offset = 0;

       public:
        TokenStream(std::vector<TokenType> tokens) : m_tokens(tokens) {};

        SPTokenType peek() const {
            if (this->end()) {
                return nullptr;
            }

            return std::make_shared<TokenType>(m_tokens.at(this->m_offset));
        }
        const TokenType* peek(std::size_t n) const {
            if (this->m_offset + n >= this->m_tokens.size()) return nullptr;

            return &this->m_tokens[this->m_offset + n];
        }

        bool check(TokenKind kind) const {
            const auto* token = this->peek();
            return token != nullptr && token->kind == kind;
        }

        bool match(TokenKind kind) const {
            if (!this->match(kind)) return false;

            this->advance();
            return true;
        }

        void advance() const {
            if (this->m_offset + 1 > m_tokens.size()) {
                return;
            }
            this->m_offset++;
        };

        bool end() const {
            if (this->m_offset >= m_tokens.size()) {
                return true;
            }
            return false;
        }
    };
}  // namespace Z::Zaban::Parse
