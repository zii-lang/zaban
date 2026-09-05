#include <Z/Zaban/Langs/CLang/Lexer.hpp>
#include <Z/Zaban/Langs/CLang/Preprocessor.hpp>
#include <Z/Zaban/Langs/CLang/TokenKind.hpp>
#include <algorithm>

namespace Z::Zaban::Langs::CLang {
    std::size_t CPreprocessor::param_index(const MacroDef&             def,
                                           const std::vector<PpToken>& body,
                                           std::size_t i) const {
        if (i >= body.size() ||
            CLexerTokenKind::Identifier != body[i].token.kind) {
            return std::size_t(-1);
        }
        const std::string name = this->spelling(body[i].token);
        const auto it = std::find(def.params.begin(), def.params.end(), name);
        return it == def.params.end()
                   ? std::size_t(-1)
                   : static_cast<std::size_t>(it - def.params.begin());
    }

    PpToken CPreprocessor::stringize(const std::vector<PpToken>& arg,
                                     const PpToken&              at) {
        std::string s = "\"";

        for (std::size_t i = 0; i < arg.size(); ++i) {
            // Whitespace before the first token is dropped and every run in
            // between collapses to one space, which the flag gives for free.
            if (0 != i && has(static_cast<TokenFlags>(arg[i].token.flags),
                              TokenFlags::WhiteSpaceBefore)) {
                s += ' ';
            }

            // Only a literal's own delimiters and backslashes get escaped.
            const bool literal =
                CLexerTokenKind::String == arg[i].token.kind ||
                CLexerTokenKind::CharLiteral == arg[i].token.kind;

            for (const char c: this->spelling(arg[i].token)) {
                if (literal && ('"' == c || '\\' == c)) s += '\\';
                s += c;
            }
        }
        s += '"';

        const std::size_t len  = s.size();
        const std::size_t base = _sources.add(std::move(s), "<stringize>");

        // The result sits where the `#` sat, so it keeps that spacing and
        // nothing else: the text is synthesized and holds no splices.
        PpToken out = at;
        out.token   = CLexerTokenType(
            CLexerTokenKind::String,
            OffsetRange<CLexerPositionType>(base, base + len),
            static_cast<std::uint16_t>(
                mask(static_cast<TokenFlags>(at.token.flags),
                     TokenFlags::AtLineStart | TokenFlags::WhiteSpaceBefore)));
        return out;
    }

    bool CPreprocessor::paste(const PpToken& a, const PpToken& b,
                              PpToken& out) {
        std::string text = this->spelling(a.token) + this->spelling(b.token);

        const std::size_t len  = text.size();
        const std::size_t base = _sources.add(std::move(text), "<paste>");

        CLexerBufferType buf = _sources.whole(base);
        CLexer           lx(buf, base);
        lx.scan();

        std::vector<CLexerTokenType> t = lx.finalize();
        std::erase_if(t, [](const CLexerTokenType& x) {
            return CLexerTokenKind::Eob == x.kind ||
                   CLexerTokenKind::Eof == x.kind;
        });

        // One token, and it has to cover the whole paste. `+` and `/` lexes
        // as two; `x` and `1.` lexes as one that stops short.
        if (1 != t.size() || t[0].range.end != base + len) {
            _errors |= CPpErrorFlags::InvalidPaste;
            return false;
        }

        out       = a;
        out.token = CLexerTokenType(
            t[0].kind, t[0].range,
            static_cast<std::uint16_t>(unset(
                static_cast<TokenFlags>(a.token.flags),
                TokenFlags::ContainsSplice | TokenFlags::ExponentPending)));
        out.hides = _hide_sets.intersect(a.hides, b.hides);
        return true;
    }

    std::vector<PpToken> CPreprocessor::substitute(
        const MacroDef& def, const std::vector<MacroArg>& args) {
        std::vector<PpToken> out;
        const auto&          body = def.body;

        // The tail of `out` came from an argument that turned out empty, so
        // a ## next to it has nothing to paste onto.
        bool placemarker = false;

        for (std::size_t i = 0; i < body.size();) {
            if (CLexerTokenKind::Hash == body[i].token.kind) {
                const std::size_t p = this->param_index(def, body, i + 1);
                if (std::size_t(-1) != p) {
                    out.push_back(this->stringize(args[p].raw, body[i]));
                    placemarker = false;
                    i += 2;
                    continue;
                }
                // A `#` not followed by a parameter is an ordinary token.
            }

            if (CLexerTokenKind::HashHash == body[i].token.kind &&
                i + 1 < body.size()) {
                const std::size_t p = this->param_index(def, body, i + 1);

                std::vector<PpToken> rhs;
                if (std::size_t(-1) != p) {
                    rhs = args[p].raw;
                } else {
                    rhs.push_back(body[i + 1]);
                }

                if (rhs.empty()) {
                    i += 2;
                    continue;  // pasting nothing leaves the left side alone
                }

                if (out.empty() || placemarker) {
                    out.insert(out.end(), rhs.begin(), rhs.end());
                } else {
                    PpToken pasted = out.back();
                    if (this->paste(out.back(), rhs[0], pasted)) {
                        out.back() = pasted;
                    } else {
                        // Already flagged. Keep both so the stream survives
                        // and the LSP still sees something at that spot.
                        out.push_back(rhs[0]);
                    }
                    out.insert(out.end(), rhs.begin() + 1, rhs.end());
                }
                placemarker = false;
                i += 2;
                continue;
            }

            const std::size_t p = this->param_index(def, body, i);
            if (std::size_t(-1) == p) {
                out.push_back(body[i]);
                placemarker = false;
                ++i;
                continue;
            }

            // An operand of ## keeps its unexpanded form. The right-hand
            // side is handled above, so only the left one lands here.
            const bool raw_form =
                i + 1 < body.size() &&
                CLexerTokenKind::HashHash == body[i + 1].token.kind;
            const std::vector<PpToken>& src =
                raw_form ? args[p].raw : args[p].expanded;

            const std::size_t first = out.size();
            out.insert(out.end(), src.begin(), src.end());

            // The parameter's spacing in the body wins over the argument's
            // spacing at the call site. An empty argument grows nothing.
            if (out.size() > first) {
                constexpr TokenFlags lead =
                    TokenFlags::AtLineStart | TokenFlags::WhiteSpaceBefore;
                out[first].token.flags &= static_cast<std::uint16_t>(~lead);
                out[first].token.flags |= static_cast<std::uint16_t>(
                    mask(static_cast<TokenFlags>(body[i].token.flags), lead));
            }

            placemarker = src.empty();
            ++i;
        }
        return out;
    }
}  // namespace Z::Zaban::Langs::CLang
