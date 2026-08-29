#include <Z/Zaban/Langs/CLang/Preprocessor.hpp>
#include <cstddef>
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

        MacroDef def;
        def.name               = this->spelling(tokens[name_idx]);
        std::size_t body_start = name_idx + 1;

        // '(' means its a function
        if (name_idx + 1 < d.end_index &&
            tokens[name_idx + 1].kind == TokenKind::LParen &&
            !has(static_cast<TokenFlags>(tokens[name_idx + 1].flags),
                 TokenFlags::WhiteSpaceBefore)) {
            def.function_like          = true;
            std::size_t j              = body_start + 1;
            bool        expected_param = true;
            while (j < d.end_index &&
                   tokens[j].kind != CLexerTokenKind::RParen) {
                if (tokens[j].kind == CLexerTokenKind::Comma) {
                    expected_param = true;
                } else if (expected_param) {
                    def.params.push_back(this->spelling(tokens[j]));
                    expected_param = false;
                } else {
                    // TODO: err
                    return;
                }
                ++j;
            }
            if (j >= d.end_index) {
                // TODO: err
                return;
            }
            // one past ')'
            body_start = j + 1;
        }

        def.body.assign(tokens.begin() + body_start,
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
    std::size_t CPreprocessor::find_invocation_paren(
        const std::vector<CLexerTokenType>& tokens, std::size_t i) const {
        const auto j = i + 1;
        if (j >= tokens.size()) return std::size_t(-1);
        if (tokens[j].kind != CLexerTokenKind::LParen) return std::size_t(-1);
        if (has(static_cast<TokenFlags>(tokens[j].flags),
                TokenFlags::DirectiveLine)) {
            return std::size_t(-1);
        }
        return j;
    }

    bool CPreprocessor::collect_arguments(
        const std::vector<CLexerTokenType>& tokens, std::size_t lparen,
        std::vector<std::vector<CLexerTokenType>>& out,
        std::size_t&                               end) const {
        out.clear();
        out.emplace_back();

        std::size_t depth = 1;

        for (std::size_t i = lparen + 1; i < tokens.size(); ++i) {
            const CLexerTokenKind k = tokens[i].kind;
            if (k == CLexerTokenKind::Eob) break;
            if (k == CLexerTokenKind::LParen) {
                ++depth;
            } else if (k == CLexerTokenKind::RParen) {
                --depth;
                if (depth == 0) {
                    end = i + 1;
                    // F() is 'one empty argument' which a zero param macro
                    // reads as 'no arg' at all
                    if (out.size() == 1 && out[0].empty()) out.clear();
                    return true;
                }
            } else if (k == CLexerTokenKind::Comma && depth == 1) {
                out.emplace_back();
                continue;
            }
            out.back().push_back(tokens[i]);
        }
        return false;
    }

    std::vector<CLexerTokenType> CPreprocessor::substitute(
        const MacroDef&                                  def,
        const std::vector<std::vector<CLexerTokenType>>& args) const {
        std::vector<CLexerTokenType> out;
        for (const auto& b: def.body) {
            if (b.kind != CLexerTokenKind::Identifier) {
                out.push_back(b);
                continue;
            }
            const auto name = this->spelling(b);
            const auto it =
                std::find(def.params.begin(), def.params.end(), name);
            if (it == def.params.end()) {
                out.push_back(b);
                continue;
            }
            const std::size_t p     = it - def.params.begin();
            const auto        first = out.size();
            out.insert(out.end(), args[p].begin(), args[p].end());
            // if args[p] is empty, out doesnt grow!
            if (out.size() > first) {
                constexpr TokenFlags lead =
                    TokenFlags::AtLineStart | TokenFlags::WhiteSpaceBefore;
                out[first].flags &= static_cast<std::uint16_t>(~lead);
                out[first].flags |= static_cast<std::uint16_t>(
                    mask(static_cast<TokenFlags>(b.flags), lead));
            }
        }
        return out;
    }

    std::size_t CPreprocessor::expand_into(
        const std::vector<CLexerTokenType>& tokens, std::size_t i,
        std::vector<CLexerTokenType>&    out,
        std::unordered_set<std::string>& active) const {
        const CLexerTokenType& t = tokens[i];
        if (t.kind != CLexerTokenKind::Identifier) {
            out.push_back(t);
            return i + 1;
        }

        const auto name = this->spelling(t);
        const auto it   = _macros.find(name);
        if (it == _macros.end() || active.contains(name)) {
            out.push_back(t);
            return i + 1;
        }

        const MacroDef&              def = it->second;
        std::vector<CLexerTokenType> replacement;
        auto                         next = i + 1;
        if (def.function_like) {
            const auto lparen = this->find_invocation_paren(tokens, i);
            if (lparen == std::size_t(-1)) {
                // a function like name without '(' is a plain ident
                out.push_back(t);
                return i + 1;
            }
            std::vector<std::vector<CLexerTokenType>> args;
            std::size_t                               end = 0;
            if (!this->collect_arguments(tokens, lparen, args, end)) {
                // TODO: err
                out.push_back(t);
                return i + 1;
            }
            if (args.size() != def.params.size()) {
                // TODO: err
                out.push_back(t);
                return i + 1;
            }
            replacement = this->substitute(def, args);
            next        = end;
        } else {
            replacement = def.body;
        }
        const std::size_t first = out.size();

        active.insert(name);
        for (std::size_t k = 0; k < replacement.size();) {
            k = this->expand_into(replacement, k, out, active);
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
        return next;
    }

    std::vector<CLexerTokenType> CPreprocessor::process(
        std::vector<CLexerTokenType> tokens) {
        std::vector<CLexerTokenType>    out;
        std::unordered_set<std::string> active;
        Directive                       d;

        for (std::size_t i = 0; i < tokens.size();) {
            if (!read_directive(tokens, i, d)) {
                i = this->expand_into(tokens, i, out, active);
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
