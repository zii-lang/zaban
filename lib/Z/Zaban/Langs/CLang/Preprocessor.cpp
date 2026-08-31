#include <Z/Zaban/Langs/CLang/Lexer.hpp>
#include <Z/Zaban/Langs/CLang/LexerTypes.hpp>
#include <Z/Zaban/Langs/CLang/Preprocessor.hpp>
#include <Z/Zaban/Langs/CLang/TokenKind.hpp>
#include <cstddef>
#include <cstdint>

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

    bool CPreprocessor::read_directive(const std::vector<PpToken>& tokens,
                                       std::size_t i, Directive& out) const {
        if (i >= tokens.size() || !this->is_directive_start(tokens[i].token)) {
            return false;
        }

        out.hash_index = i;

        // The directive line ends at the next token that starts a line.
        std::size_t j = i + 1;
        while (j < tokens.size() &&
               tokens[j].token.kind != CLexerTokenKind::Eob &&
               !has(static_cast<TokenFlags>(tokens[j].token.flags),
                    TokenFlags::AtLineStart)) {
            ++j;
        }
        out.end_index = j;

        if (i + 1 < out.end_index) {
            out.keyword_kind = tokens[i + 1].token.kind;
            out.keyword      = this->spelling(tokens[i + 1].token);
        } else {
            out.keyword_kind = CLexerTokenKind::Dummy;
            out.keyword      = {};
        }
        return true;
    }

    void CPreprocessor::handle_undef(const std::vector<PpToken>& tokens,
                                     const Directive&            d) {
        const std::size_t name_idx = d.hash_index + 2;
        if (name_idx >= d.end_index) return;
        _macros.erase(this->spelling(tokens[name_idx].token));
    }

    void CPreprocessor::handle_define(const std::vector<PpToken>& tokens,
                                      const Directive&            d) {
        const std::size_t name_idx = d.hash_index + 2;
        if (name_idx >= d.end_index) return;

        MacroDef def;
        def.name = this->spelling(tokens[name_idx].token);

        std::size_t body_start = name_idx + 1;

        // A `(` touching the name means function-like. With space before it,
        // the paren is just the first token of an object-like body.
        if (body_start < d.end_index &&
            tokens[body_start].token.kind == CLexerTokenKind::LParen &&
            !has(static_cast<TokenFlags>(tokens[body_start].token.flags),
                 TokenFlags::WhiteSpaceBefore)) {
            def.function_like = true;

            std::size_t j              = body_start + 1;
            bool        expected_param = true;
            while (j < d.end_index &&
                   tokens[j].token.kind != CLexerTokenKind::RParen) {
                if (tokens[j].token.kind == CLexerTokenKind::Comma) {
                    expected_param = true;
                } else if (expected_param) {
                    def.params.push_back(this->spelling(tokens[j].token));
                    expected_param = false;
                } else {
                    return;  // TODO: MalformedDirective
                }
                ++j;
            }
            if (j >= d.end_index) {
                return;  // TODO: MalformedDirective
            }
            body_start = j + 1;  // one past `)`
        }

        def.body.assign(tokens.begin() + body_start,
                        tokens.begin() + d.end_index);

        // Body tokens are no longer part of a directive line, so the
        // replacement looks like ordinary text at the call site.
        constexpr TokenFlags strip =
            TokenFlags::DirectiveLine | TokenFlags::AtLineStart;
        for (auto& b: def.body) {
            b.token.flags &= static_cast<std::uint16_t>(~strip);
        }

        _macros[def.name] = std::move(def);
    }
    std::size_t CPreprocessor::find_invocation_paren(
        const std::vector<PpToken>& tokens, std::size_t i) const {
        const std::size_t j = i + 1;
        if (j >= tokens.size()) return std::size_t(-1);
        if (tokens[j].token.kind != CLexerTokenKind::LParen) {
            return std::size_t(-1);
        }
        if (has(static_cast<TokenFlags>(tokens[j].token.flags),
                TokenFlags::DirectiveLine)) {
            return std::size_t(-1);
        }
        return j;
    }

    bool CPreprocessor::collect_arguments(
        const std::vector<PpToken>& tokens, std::size_t lparen,
        std::vector<std::vector<PpToken>>& out, std::size_t& end) const {
        out.clear();
        out.emplace_back();

        std::size_t depth = 1;

        for (std::size_t i = lparen + 1; i < tokens.size(); ++i) {
            const CLexerTokenKind k = tokens[i].token.kind;

            if (k == CLexerTokenKind::Eob) break;

            if (k == CLexerTokenKind::LParen) {
                ++depth;
            } else if (k == CLexerTokenKind::RParen) {
                --depth;
                if (0 == depth) {
                    end = i + 1;
                    return true;
                }
            } else if (k == CLexerTokenKind::Comma && 1 == depth) {
                out.emplace_back();
                continue;
            }
            out.back().push_back(tokens[i]);
        }
        return false;
    }

    std::vector<PpToken> CPreprocessor::substitute(
        const MacroDef&                          def,
        const std::vector<std::vector<PpToken>>& args) const {
        std::vector<PpToken> out;

        for (const auto& b: def.body) {
            if (b.token.kind != CLexerTokenKind::Identifier) {
                out.push_back(b);
                continue;
            }

            const std::string name = this->spelling(b.token);
            const auto        it =
                std::find(def.params.begin(), def.params.end(), name);
            if (it == def.params.end()) {
                out.push_back(b);
                continue;
            }

            const std::size_t p     = it - def.params.begin();
            const std::size_t first = out.size();
            out.insert(out.end(), args[p].begin(), args[p].end());

            // The parameter's spacing in the body wins over the argument's
            // spacing at the call site. An empty argument grows nothing.
            if (out.size() > first) {
                constexpr TokenFlags lead =
                    TokenFlags::AtLineStart | TokenFlags::WhiteSpaceBefore;
                out[first].token.flags &= static_cast<std::uint16_t>(~lead);
                out[first].token.flags |= static_cast<std::uint16_t>(
                    mask(static_cast<TokenFlags>(b.token.flags), lead));
            }
        }
        return out;
    }

    std::size_t CPreprocessor::expand_into(const std::vector<PpToken>& tokens,
                                           std::size_t                 i,
                                           std::vector<PpToken>&       out) {
        const PpToken& t = tokens[i];

        if (t.token.kind != CLexerTokenKind::Identifier) {
            out.push_back(t);
            return i + 1;
        }

        const std::string name = this->spelling(t.token);
        const auto        it   = _macros.find(name);
        if (it == _macros.end() || _hide_sets.contains(t.hides, name)) {
            out.push_back(t);
            return i + 1;
        }

        const MacroDef&      def = it->second;
        std::vector<PpToken> replacement;
        std::size_t          next = i + 1;
        Pp::HideSetId        hs   = Pp::HideSetTable::Empty;

        if (def.function_like) {
            const std::size_t lparen = this->find_invocation_paren(tokens, i);
            if (lparen == std::size_t(-1)) {
                // A function-like name without `(` is a plain identifier.
                out.push_back(t);
                return i + 1;
            }

            std::vector<std::vector<PpToken>> args;
            std::size_t                       end = 0;
            if (!this->collect_arguments(tokens, lparen, args, end)) {
                out.push_back(t);
                return i + 1;  // TODO: MalformedDirective
            }
            // `F()` is one empty argument. A zero-parameter macro reads that
            // as no arguments at all. a one-parameter macro reads it as one
            // empty argument
            if (def.params.empty() && 1 == args.size() && args[0].empty()) {
                args.clear();
            }
            if (args.size() != def.params.size()) {
                out.push_back(t);
                return i + 1;  // TODO: MacroArity
            }

            // Arguments are expanded in the caller's context, before
            // substitution. This is why the hide set has to live on tokens:
            // the macro is not yet hidden here, but it will be during the
            // rescan below.
            for (auto& arg: args) {
                std::vector<PpToken> expanded;
                for (std::size_t k = 0; k < arg.size();) {
                    k = this->expand_into(arg, k, expanded);
                }
                arg = std::move(expanded);
            }

            replacement = this->substitute(def, args);
            next        = end;

            // The result hides the intersection of the name's and the closing
            // paren's sets, plus this macro.
            hs = _hide_sets.add(
                _hide_sets.intersect(t.hides, tokens[end - 1].hides), name);
        } else {
            replacement = def.body;
            hs          = _hide_sets.add(t.hides, name);
        }

        // Argument tokens arrive with their own sets from pre-expansion, so
        // merge rather than overwrite.
        for (auto& r: replacement) {
            r.hides = _hide_sets.merge(r.hides, hs);
        }

        const std::size_t first = out.size();
        for (std::size_t k = 0; k < replacement.size();) {
            k = this->expand_into(replacement, k, out);
        }

        // Body tokens carry the spacing they had on the #define line, which
        // says nothing about the call site. Move the invocation's position
        // flags onto the first replacement token; the rest keep their own.
        if (out.size() > first) {
            constexpr TokenFlags lead =
                TokenFlags::AtLineStart | TokenFlags::WhiteSpaceBefore;
            out[first].token.flags &= static_cast<std::uint16_t>(~lead);
            out[first].token.flags |= static_cast<std::uint16_t>(
                mask(static_cast<TokenFlags>(t.token.flags), lead));
        }
        return next;
    }

    std::vector<CLexerTokenType> CPreprocessor::process(
        std::vector<CLexerTokenType> tokens) {
        std::vector<PpToken> in;
        in.reserve(tokens.size());
        for (auto& t: tokens) {
            in.push_back(PpToken{std::move(t), Pp::HideSetTable::Empty});
        }

        std::vector<PpToken> out;
        out.reserve(in.size());

        Directive d;
        for (std::size_t i = 0; i < in.size();) {
            if (!this->read_directive(in, i, d)) {
                i = this->expand_into(in, i, out);
                continue;
            }

            for (std::size_t k = d.hash_index; k < d.end_index; ++k) {
                in[k].token.flags |=
                    static_cast<std::uint16_t>(TokenFlags::DirectiveLine);
            }

            if (d.keyword == "define") {
                this->handle_define(in, d);
            } else if (d.keyword == "undef") {
                this->handle_undef(in, d);
            }

            // Directive lines are kept and marked, never expanded.
            out.insert(out.end(), in.begin() + d.hash_index,
                       in.begin() + d.end_index);
            i = d.end_index;
        }

        std::vector<CLexerTokenType> result;
        result.reserve(out.size());
        for (auto& p: out) {
            result.push_back(std::move(p.token));
        }
        return result;
    }

}  // namespace Z::Zaban::Langs::CLang
