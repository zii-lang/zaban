#include <Z/Zaban/Langs/CLang/Preprocessor.hpp>
#include <cstdint>

#include "Z/Zaban/Langs/CLang/Lexer.hpp"
#include "Z/Zaban/Langs/CLang/LexerTypes.hpp"

namespace Z::Zaban::Langs::CLang {
    bool CPreprocessor::is_directive_start(const CLexerTokenType& t) const {
        return t.kind == CLexerTokenKind::Hash &&
               has(static_cast<TokenFlags>(t.flags), TokenFlags::AtLineStart);
    }

    std::string CPreprocessor::spelling(const CLexerTokenType& t) const {
        const CLexerBufferType text =
            _source.substr(t.range.begin, t.range.end - t.range.begin);

        return has(static_cast<TokenFlags>(t.flags), TokenFlags::ContainsSplice)
                   ? unsplice(text)
                   : std::string(text);
    }

    bool CPreprocessor::read_directive(
        const std::vector<CLexerTokenType>& tokens, std::size_t i,
        Directive& out) const {
        if (i >= tokens.size() || !is_directive_start(tokens[i])) return false;

        out.hash_index = i;
        // directive line ends at the nex token that starts a line
        std::size_t j = i + 1;
        while (j < tokens.size() && tokens[j].kind != CLexerTokenKind::Eob &&
               !has(static_cast<TokenFlags>(tokens[j].flags),
                    TokenFlags::AtLineStart)) {
            ++j;
        }
        out.end_index = j;

        if (i + 1 < out.end_index) {
            out.keyword_kind = tokens[i + 1].kind;
            out.keyword      = spelling(tokens[i + 1]);
        } else {
            out.keyword_kind = CLexerTokenKind::Dummy;
            out.keyword      = {};
        }
        return true;
    }
    std::vector<CLexerTokenType> CPreprocessor::process(
        std::vector<CLexerTokenType> tokens) {
        Directive d;
        for (std::size_t i = 0; i < tokens.size();) {
            if (!read_directive(tokens, i, d)) {
                ++i;
                continue;
            }
            for (std::size_t k = d.hash_index; k < d.end_index; ++k) {
                tokens[k].flags |=
                    static_cast<std::uint16_t>(TokenFlags::DirectiveLine);
            }
            i = d.end_index;
        }
        return tokens;
    }
}  // namespace Z::Zaban::Langs::CLang
