#pragma once

#include <Z/Zaban/Lex/Token.hpp>
#include <vector>

namespace Z::Zaban::Parse {
    template<typename TokenKind, typename OffsetType>
    class TokenStream {
        using TokenType   = Token<TokenKind, OffsetType>;
        using SPTokenType = std::shared_ptr<TokenType>;

       private:
        std::vector<TokenType> m_tokens;
        OffsetType             m_offset = 0;

       public:
        TokenStream(std::vector<TokenType> tokens) : m_tokens(tokens) {};

        SPTokenType peek() {
            if (this->end()) {
                return nullptr;
            }

            return std::make_shared<TokenType>(m_tokens.at(this->m_offset));
        };

        void advance() {
            if (this->m_offset + 1 > m_tokens.size()) {
                return;
            }
            this->m_offset++;
        };

        bool end() {
            if (this->m_offset >= m_tokens.size()) {
                return true;
            }
            return false;
        }
    };
}  // namespace Z::Zaban::Parse
