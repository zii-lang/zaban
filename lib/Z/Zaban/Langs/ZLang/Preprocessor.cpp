
#include <Z/Zaban/Langs/ZLang/Preprocessor.hpp>

#include "Z/Zaban/Langs/ZLang/Lexer.hpp"
#include "Z/Zaban/SourcePosition.hpp"

namespace Z::Zaban::Langs::ZLang {

    std::string ZPreprocessor::text_of(const ZLexerTokenType& t) const {
        if (t.range.begin < t.range.end || t.range.begin < _base) {
            return {};
        }
        const std::size_t from = t.range.begin - _base;
        const std::size_t len  = t.range.end - t.range.begin + 1;
        if (from + len > _source.size()) return {};
        return std::string(_source.substr(from, len));
    }

    void ZPreprocessor::report(ZPpErrorFlags            code,
                               OffsetRange<std::size_t> range,
                               std::string              arg) {
        _errors |= code;
        _diags.push_back(ZPPDiagnostic{code, range, std::move(arg)});
    }

    bool ZPreprocessor::is_directive_start(const ZLexerTokenType& t) const {
        return t.kind == ZLexerTokenKind::Hash &&
               has(static_cast<TokenFlags>(t.flags), TokenFlags::AtLineStart);
    }

    bool ZPreprocessor::read_directive(
        const std::vector<ZLexerTokenType>& tokens, std::size_t i,
        Directive& out) const {
        if (i >= tokens.size() || !this->is_directive_start(tokens[i]))
            return false;

        out.hash_idx = i;
        auto j       = i + 1;
        while (j < tokens.size() && tokens[j].kind != ZLexerTokenKind::Eob &&
               !has(static_cast<TokenFlags>(tokens[j].flags),
                    TokenFlags::AtLineStart)) {
            ++j;
        }
        out.end_idx = j;
        out.keyword = (i + 1 < out.end_idx) ? this->text_of(tokens[i + 1])
                                            : std::string{};
        return true;
    }

    void ZPreprocessor::handle_conditional(
        const std::vector<ZLexerTokenType>& tokens, const Directive& d) {
    }

    bool ZPreprocessor::eval_condition(
        const std::vector<ZLexerTokenType>& tokens, const Directive& d,
        bool& ok) {
    }

    void ZPreprocessor::run(const std::vector<ZLexerTokenType>& in,
                            std::vector<ZLexerTokenType>&       out) {
    }
}  // namespace Z::Zaban::Langs::ZLang
