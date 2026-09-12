#pragma once

#include <Z/Zaban/Lex/LexerError.hpp>
#include <vector>

namespace Z::Zaban::Pp {
    /** @brief Interface every preprocessor implements.
     *
     * A preprocessor consumes a pp-token stream produced by a lexer and
     * returns the post-directive stream. It never reads source bytes
     * sequentially; spelling is recovered through each token's range.
     *
     * @tparam T Token type produced by the corresponding lexer.
     */
    template<typename T>
    class IPreprocessor {
       public:
        virtual ~IPreprocessor() = default;

        virtual std::vector<T> process(std::vector<T> tokens) = 0;
    };
}  // namespace Z::Zaban::Pp
