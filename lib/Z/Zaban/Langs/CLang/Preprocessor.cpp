#include <Z/Zaban/Langs/CLang/Preprocessor.hpp>
#include <cstdint>
#include <unordered_set>

#include "Z/Zaban/Langs/CLang/Lexer.hpp"
#include "Z/Zaban/Langs/CLang/LexerTypes.hpp"
#include "Z/Zaban/Langs/CLang/TokenKind.hpp"

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

    void CPreprocessor::handle_undef(const std::vector<CLexerTokenType>& tokens,
                                     const Directive&                    d) {
        const auto name_idx = d.hash_index + 2;
        if (name_idx >= d.end_index) return;
        _macros.erase(this->spelling(tokens[name_idx]));
    }

    void CPreprocessor::handle_define(
        const std::vector<CLexerTokenType>& tokens, const Directive& d) {
        const std::size_t name_idx = d.hash_index + 2;
        if (name_idx >= d.end_index) return;

        // '(' means its a function
        // TODO:
        if (name_idx + 1 < d.end_index &&
            tokens[name_idx + 1].kind == TokenKind::LParen &&
            !has(static_cast<TokenFlags>(tokens[name_idx + 1].flags),
                 TokenFlags::WhiteSpaceBefore)) {
            return;
        }

        MacroDef def;
        def.name = this->spelling(tokens[name_idx]);
        def.body.assign(tokens.begin() + name_idx + 1,
                        tokens.begin() + d.end_index);
        // body tokens should no longer be marked as part of the directive line
        // so the replacement looks like ordinary txt
        constexpr TokenFlags strip =
            TokenFlags::DirectiveLine | TokenFlags::AtLineStart;
        for (auto& b: def.body) {
            b.flags &= static_cast<std::uint16_t>(~strip);
        }
        _macros[def.name] = std::move(def);
    }

    void CPreprocessor::expand_into(
        const CLexerTokenType& t, std::vector<CLexerTokenType>& out,
        std::unordered_set<std::string>& active) const {
        if (t.kind != CLexerTokenKind::Identifier) {
            out.push_back(t);
            return;
        }

        const auto name = this->spelling(t);
        const auto it   = _macros.find(name);
        if (it == _macros.end() || active.contains(name)) {
            out.push_back(t);
            return;
        }

        const auto first = out.size();

        active.insert(name);
        for (const auto& b: it->second.body) {
            this->expand_into(b, out, active);
        }
        active.erase(name);
        // body tokens carry the spacing they had in the #define line, which
        // says nothing about the call site. so we should mov AtLineStart and
        // WhiteSpaceBefore from the invocation onto the first replacement
        // token. shouldnt have to change the others
        // the rest keep their own spacing
        if (out.size() > first) {
            constexpr TokenFlags lead =
                TokenFlags::AtLineStart | TokenFlags::WhiteSpaceBefore;
            out[first].flags &= static_cast<uint16_t>(~lead);
            out[first].flags |= static_cast<std::uint16_t>(
                mask(static_cast<TokenFlags>(t.flags), lead));
        }
    }

    std::vector<CLexerTokenType> CPreprocessor::process(
        std::vector<CLexerTokenType> tokens) {
        std::vector<CLexerTokenType>    out;
        std::unordered_set<std::string> active;
        Directive                       d;

        for (std::size_t i = 0; i < tokens.size();) {
            if (!read_directive(tokens, i, d)) {
                expand_into(tokens[i], out, active);
                ++i;
                continue;
            }
            for (std::size_t k = d.hash_index; k < d.end_index; ++k) {
                tokens[k].flags |=
                    static_cast<std::uint16_t>(TokenFlags::DirectiveLine);
            }
            if (d.keyword == "define") {
                this->handle_define(tokens, d);
            } else if (d.keyword == "undef") {
                this->handle_undef(tokens, d);
            }
            out.insert(out.end(), tokens.begin() + d.hash_index,
                       tokens.begin() + d.end_index);
            i = d.end_index;
        }
        return out;
    }

}  // namespace Z::Zaban::Langs::CLang
