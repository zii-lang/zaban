#include <Z/Zaban/Langs/CLang/Lexer.hpp>
#include <Z/Zaban/Langs/CLang/LexerTypes.hpp>
#include <Z/Zaban/Langs/CLang/Preprocessor.hpp>
#include <Z/Zaban/Langs/CLang/TokenKind.hpp>
#include <algorithm>
#include <cstddef>
#include <cstdint>

#include "Z/Zaban/SourcePosition.hpp"

namespace Z::Zaban::Langs::CLang {

    void CPreprocessor::report(CPpErrorFlags            code,
                               OffsetRange<std::size_t> range,
                               std::string              arg) {
        _errors |= code;
        _diags.push_back(PPDiagnostic{code, range, std::move(arg), _files});
    }

    bool CPreprocessor::same_definition(const MacroDef& a,
                                        const MacroDef& b) const {
        if (a.function_like != b.function_like) return false;
        if (a.params != b.params) return false;
        if (a.body.size() != b.body.size()) return false;

        for (std::size_t i = 0; i < a.body.size(); ++i) {
            if (a.body[i].token.kind != b.body[i].token.kind) return false;
            if (this->spelling(a.body[i].token) !=
                this->spelling(b.body[i].token)) {
                return false;
            }
            // any white space run is one separation but must be present in both
            // or abset in both
            const bool sa = has(static_cast<TokenFlags>(a.body[i].token.flags),
                                TokenFlags::WhiteSpaceBefore);
            const bool sb = has(static_cast<TokenFlags>(b.body[i].token.flags),
                                TokenFlags::WhiteSpaceBefore);
            if (sa != sb && i != 0) return false;
        }
        return true;
    }

    bool CPreprocessor::is_directive_start(const CLexerTokenType& t) const {
        return t.kind == CLexerTokenKind::Hash &&
               has(static_cast<TokenFlags>(t.flags), TokenFlags::AtLineStart);
    }

    std::string CPreprocessor::spelling(const CLexerTokenType& t) const {
        const CLexerBufferType text = _sources.text(t.range.begin, t.range.end);

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
        if (name_idx >= d.end_index) {
            report(CPpErrorFlags::MalformedDirective,
                   tokens[d.hash_index].token.range);
            return;
        };

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
                    if (CLexerTokenKind::Identifier != tokens[j].token.kind) {
                        report(CPpErrorFlags::MalformedDirective,
                               tokens[j].token.range,
                               spelling(tokens[j].token));
                        return;
                    }
                    std::string p = this->spelling(tokens[j].token);
                    if (std::find(def.params.begin(), def.params.end(), p) !=
                        def.params.end()) {
                        report(CPpErrorFlags::DuplicateParam,
                               tokens[j].token.range, p);
                        return;
                    }
                    def.params.push_back(std::move(p));
                    expected_param = false;
                } else {
                    report(CPpErrorFlags::MalformedDirective,
                           tokens[j].token.range, def.name);
                    return;
                }
                ++j;
            }
            if (j >= d.end_index) {
                report(CPpErrorFlags::MalformedDirective,
                       tokens[body_start].token.range, def.name);
                return;
            }
            body_start = j + 1;  // one past `)`
        }

        def.body.assign(tokens.begin() + body_start,
                        tokens.begin() + d.end_index);

        if (def.function_like) {
            for (std::size_t k = 0; k < def.body.size(); ++k) {
                if (def.body[k].token.kind != CLexerTokenKind::Hash) continue;
                if (this->param_index(def, def.body, k + 1) ==
                    std::size_t(-1)) {
                    report(CPpErrorFlags::InvalidStringize,
                           def.body[k].token.range, def.name);
                    return;
                }
            }
        }

        if (!def.body.empty() &&
            (def.body.front().token.kind == CLexerTokenKind::HashHash ||
             def.body.back().token.kind == CLexerTokenKind::HashHash)) {
            report(CPpErrorFlags::InvalidPaste, def.body.front().token.range,
                   def.name);
            return;
        }

        // Body tokens are no longer part of a directive line, so the
        // replacement looks like ordinary text at the call site.
        constexpr TokenFlags strip =
            TokenFlags::DirectiveLine | TokenFlags::AtLineStart;
        for (auto& b: def.body) {
            b.token.flags &= static_cast<std::uint16_t>(~strip);
        }

        // to guard against redef
        const auto prev = _macros.find(def.name);
        if (prev != _macros.end() &&
            !this->same_definition(prev->second, def)) {
            report(CPpErrorFlags::MacroRedefined, tokens[name_idx].token.range,
                   def.name);
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

    bool CPreprocessor::collect_arguments(const std::vector<PpToken>& tokens,
                                          std::size_t                 lparen,
                                          std::vector<MacroArg>&      out,
                                          std::size_t& end) const {
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
            out.back().raw.push_back(tokens[i]);
        }
        return false;
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

            std::vector<MacroArg> args;
            std::size_t           end = 0;
            if (!this->collect_arguments(tokens, lparen, args, end)) {
                report(CPpErrorFlags::UnterminatedArgs,
                       tokens[lparen].token.range, name);
                out.push_back(t);
                return i + 1;
            }
            // `F()` is one empty argument. A zero-parameter macro reads that
            // as no arguments at all. a one-parameter macro reads it as one
            // empty argument
            if (def.params.empty() && 1 == args.size() && args[0].raw.empty()) {
                args.clear();
            }
            if (args.size() != def.params.size()) {
                report(CPpErrorFlags::MacroArity, t.token.range, name);
                out.push_back(t);
                return i + 1;
            }

            // Arguments are expanded in the caller's context, before
            // substitution. This is why the hide set has to live on tokens:
            // the macro is not yet hidden here, but it will be during the
            // rescan below.
            for (auto& arg: args) {
                for (std::size_t k = 0; k < arg.raw.size();) {
                    k = this->expand_into(arg.raw, k, arg.expanded);
                }
            }

            replacement = this->substitute(def, args);
            next        = end;

            // The result hides the intersection of the name's and the closing
            // paren's sets, plus this macro.
            hs = _hide_sets.add(
                _hide_sets.intersect(t.hides, tokens[end - 1].hides), name);
        } else {
            replacement = this->substitute(def, {});
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

        _files.push_back(_sources.name(0));
        this->run(in, out);
        _files.pop_back();

        if (!_cond.empty())
            report(CPpErrorFlags::UnterminatedIf, _cond.front().opened_at);

        std::vector<CLexerTokenType> result;
        result.reserve(out.size());
        for (auto& p: out) {
            result.push_back(std::move(p.token));
        }
        return result;
    }

    void CPreprocessor::run(std::vector<PpToken>& in,
                            std::vector<PpToken>& out) {
        Directive d;
        for (std::size_t i = 0; i < in.size();) {
            if (!this->read_directive(in, i, d)) {
                if (this->skipping()) {
                    in[i].token.flags |=
                        static_cast<std::uint16_t>(TokenFlags::Skipped);
                    out.push_back(in[i]);
                    ++i;
                } else {
                    i = this->expand_into(in, i, out);
                }
                continue;
            }

            for (std::size_t k = d.hash_index; k < d.end_index; ++k) {
                in[k].token.flags |=
                    static_cast<std::uint16_t>(TokenFlags::DirectiveLine);
            }

            const bool cond = is_conditional(d.keyword);
            const bool dead = this->skipping();

            // A conditional directive delimits the dead group rather than
            // belonging to it, so it stays unmarked.
            if (dead && !cond) {
                for (std::size_t k = d.hash_index; k < d.end_index; ++k) {
                    in[k].token.flags |=
                        static_cast<std::uint16_t>(TokenFlags::Skipped);
                }
            }

            // Directive lines are kept and marked, never expanded. The line
            // goes out before the header's tokens do.
            out.insert(out.end(), in.begin() + d.hash_index,
                       in.begin() + d.end_index);

            if (cond) {
                this->handle_conditional(in, d);
            } else if (!dead) {
                if ("define" == d.keyword) {
                    this->handle_define(in, d);
                } else if ("undef" == d.keyword) {
                    this->handle_undef(in, d);
                } else if ("include" == d.keyword) {
                    this->handle_include(in, d, out);
                } else if (d.keyword == "pragma") {
                    this->handle_pragma(in, d);
                } else if (!d.keyword.empty()) {
                    report(CPpErrorFlags::UnknownDirective,
                           in[d.hash_index + 1].token.range, d.keyword);
                }
            }

            i = d.end_index;
        }
    }

}  // namespace Z::Zaban::Langs::CLang
