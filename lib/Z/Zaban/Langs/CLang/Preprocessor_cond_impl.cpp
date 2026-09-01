
#include <cstddef>
#include <cstdint>
#include <limits>

#include "Z/Zaban/Langs/CLang/Lexer.hpp"
#include "Z/Zaban/Langs/CLang/Preprocessor.hpp"
namespace Z::Zaban::Langs::CLang {
    namespace {
        /// a pptoken of #if expr reduced to what the parser needs
        struct CondTerm {
            CLexerTokenKind kind;
            std::string     text;
        };
        /* The value of a subexpr. #if arith runs in the widest int type and
         * unsigned wins. so the bit pattern + a signedness flag is the whole
         * type system
         */
        struct Value {
            std::uintmax_t bits        = 0;
            bool           is_unsigned = false;
        };
        constexpr std::uintmax_t IntWidth =
            std::numeric_limits<std::uintmax_t>::digits;
        Value make_bool(bool b) {
            return Value{b ? 1u : 0u, false};
        }

        /// a keyword has no macro def so #if reads it like its an undefined
        /// name. true and false are exceptions
        bool is_name(CLexerTokenKind k) {
            return k == CLexerTokenKind::Identifier ||
                   (k >= CLexerTokenKind::Alignas &&
                    k <= CLexerTokenKind::While);
        }
        int digit_value(char c) {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'a' && c <= 'f') return c - 'a' + 10;
            if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        }
        Value number_value(const std::string& s, bool& ok) {
            Value       v;
            int         base = 10;
            std::size_t i    = 0;
            if (s.size() > 1 && s[0] == '0') {
                if (s[1] == 'x' || s[1] == 'X') {
                    base = 16;
                    i    = 2;
                } else if (s[1] == 'b' || s[1] == 'B') {
                    base = 2;
                    i    = 2;
                } else {
                    base = 8;
                }
            }
            const std::size_t first = i;
            for (; i < s.size(); ++i) {
                const int d = digit_value(s[i]);
                if (d < 0 || d >= base) break;
                v.bits = v.bits * static_cast<std::uintmax_t>(base) +
                         static_cast<std::uintmax_t>(d);
            }
            if (i == first) {
                ok = false;
                return v;
            }
            // what is left has to be int suffix
            int l = 0;
            for (; i < s.size(); ++i) {
                const char c = static_cast<char>(s[i] | 0x20);
                if (c == 'u' && !v.is_unsigned) {
                    v.is_unsigned = true;
                } else if (c == 'l' && l < 2) {
                    ++l;
                } else {
                    ok = false;
                    break;
                }
            }
            // Too wide for intmax_t so it is unsigned whether
            if (v.bits > static_cast<std::uintmax_t>(
                             std::numeric_limits<std::intmax_t>::max())) {
                v.is_unsigned = true;
            }
            return v;
        }

        std::uintmax_t char_value(const std::string& s, bool& ok) {
            if (s.size() < 3 || s.front() != '\'' || s.back() != '\'') {
                ok = false;
                return 0;
            }
            std::uintmax_t v     = 0;
            std::uintmax_t last  = 0;
            std::size_t    count = 0;

            for (std::size_t i = 1; i + 1 < s.size();) {
                std::uintmax_t c = 0;
                if (s[i] == '\\') {
                    c = static_cast<std::uintmax_t>(
                        static_cast<unsigned char>(s[i]));
                    ++i;
                } else if (++i, i + 1 >= s.size()) {
                    ok = false;
                    break;
                } else if (s[i] == 'x') {
                    ++i;
                    const std::size_t start = i;
                    for (; i + 1 < s.size() && digit_value(s[i]) >= 0 &&
                           digit_value(s[i]) < 16;
                         ++i) {
                        c = c * 16 +
                            static_cast<std::uintmax_t>(digit_value(s[i]));
                    }
                    if (i == start) ok = false;
                } else if (s[i] >= '0' && s[i] <= '7') {
                    for (int n = 0; n < 3 && i + 1 < s.size() && s[i] >= '0' &&
                                    s[i] <= '7';
                         ++n, ++i) {
                        c = c * 8 + static_cast<std::uintmax_t>(s[i] - '0');
                    }
                } else {
                    switch (s[i]) {
                        case 'n':
                            c = '\n';
                            break;
                        case 't':
                            c = '\t';
                            break;
                        case 'r':
                            c = '\r';
                            break;
                        case 'a':
                            c = '\a';
                            break;
                        case 'b':
                            c = '\b';
                            break;
                        case 'f':
                            c = '\f';
                            break;
                        case 'v':
                            c = '\v';
                            break;
                        case 'e':
                            c = 27;
                            break;
                        case '\\':
                        case '\'':
                        case '"':
                        case '?':
                            c = static_cast<std::uintmax_t>(s[i]);
                            break;
                        default:
                            ok = false;
                            c  = static_cast<std::uintmax_t>(s[i]);
                    }
                    ++i;
                }
                last = c & 0xFF;
                v    = (v << 8) | last;
                ++count;
            }
            if (count == 0) ok = false;
            return count == 1
                       ? static_cast<std::uintmax_t>(static_cast<std::intmax_t>(
                             static_cast<signed char>(last)))
                       : v;
        }

        /// Recursive descent over the expanded #if line
        class CondEval {
           public:
            explicit CondEval(const std::vector<CondTerm>& terms) :
                _terms(terms) {
            }
            bool run(bool& ok) {
                const Value v = this->conditional();
                if (_index != _terms.size()) _ok = false;
                ok = _ok;
                return v.bits != 0;
            }

           private:
            const std::vector<CondTerm> _terms;
            std::size_t                 _index = 0;
            bool                        _ok    = true;
            // false inside a branch the ooperators already decided
            // '1/0' in '#if 0 && 1/0' is not an err!
            bool _live = true;

            CLexerTokenKind peek() const {
                return _index < _terms.size() ? _terms[_index].kind
                                              : CLexerTokenKind::Eof;
            }
            bool eat(CLexerTokenKind k) {
                if (this->peek() != k) return false;
                ++_index;
                return true;
            }
            static int precedence(CLexerTokenKind k) {
                switch (k) {
                    case CLexerTokenKind::Pipe:
                        return 1;
                    case CLexerTokenKind::Caret:
                        return 2;
                    case CLexerTokenKind::Amp:
                        return 3;
                    case CLexerTokenKind::EqualEqual:
                    case CLexerTokenKind::ExclamEqual:
                        return 4;
                    case CLexerTokenKind::Lesser:
                    case CLexerTokenKind::Greater:
                    case CLexerTokenKind::LesserEqual:
                    case CLexerTokenKind::GreaterEqual:
                        return 5;
                    case CLexerTokenKind::LesserLesser:
                    case CLexerTokenKind::GreaterGreater:
                        return 6;
                    case CLexerTokenKind::Plus:
                    case CLexerTokenKind::Minus:
                        return 7;
                    case CLexerTokenKind::Asterisk:
                    case CLexerTokenKind::Slash:
                    case CLexerTokenKind::Percent:
                        return 8;
                    default:
                        return 0;
                }
            }
            Value conditional() {
                const Value c = this->logical_or();
                if (!this->eat(CLexerTokenKind::Question)) return c;
                const bool  taken = c.bits != 0;
                const Value a     = this->branch(taken);
                if (!this->eat(CLexerTokenKind::Colon)) {
                    _ok = false;
                    return Value{};
                }
                const Value b = this->branch(!taken);
                Value       r = taken ? a : b;
                r.is_unsigned = a.is_unsigned || b.is_unsigned;
                return r;
            }

            Value branch(bool live) {
                const bool saved = _live;
                _live            = _live && live;
                const Value v    = this->conditional();
                _live            = saved;
                return v;
            }
            Value logical_or() {
                Value l = this->logical_and();
                while (this->eat(CLexerTokenKind::PipePipe)) {
                    const bool saved = _live;
                    _live            = _live && l.bits == 0;
                    const Value r    = this->logical_and();
                    _live            = saved;
                    l                = make_bool(l.bits != 0 || r.bits != 0);
                }
                return l;
            }
            Value logical_and() {
                Value l = this->binary(1);
                while (this->eat(CLexerTokenKind::AmpAmp)) {
                    const bool saved = _live;
                    _live            = _live && 0 != l.bits;
                    const Value r    = this->binary(1);
                    _live            = saved;
                    l                = make_bool(0 != l.bits && 0 != r.bits);
                }
                return l;
            }
            Value binary(int min_prec) {
                Value l = this->unary();
                for (int p = precedence(this->peek()); p >= min_prec && p > 0;
                     p     = precedence(this->peek())) {
                    const CLexerTokenKind op = _terms[_index++].kind;
                    const Value           r  = this->binary(p + 1);
                    l                        = this->apply(op, l, r);
                }
                return l;
            }
            Value unary() {
                switch (this->peek()) {
                    case CLexerTokenKind::Plus:
                        ++_index;
                        return this->unary();
                    case CLexerTokenKind::Minus: {
                        ++_index;
                        const Value v = this->unary();
                        return Value{0u - v.bits, v.is_unsigned};
                    }
                    case CLexerTokenKind::Tilde: {
                        ++_index;
                        const Value v = this->unary();
                        return Value{~v.bits, v.is_unsigned};
                    }
                    case CLexerTokenKind::Exclam:
                        ++_index;
                        return make_bool(0 == this->unary().bits);
                    default:
                        return this->primary();
                }
            }
            Value primary() {
                if (_index >= _terms.size()) {
                    _ok = false;
                    return Value{};
                }
                const CondTerm& t = _terms[_index++];

                switch (t.kind) {
                    case CLexerTokenKind::Numeric:
                        return number_value(t.text, _ok);
                    case CLexerTokenKind::CharLiteral:
                        return Value{char_value(t.text, _ok), false};
                    case CLexerTokenKind::True:
                        return Value{1, false};
                    case CLexerTokenKind::False:
                        return Value{0, false};
                    case CLexerTokenKind::LParen: {
                        const Value v = this->conditional();
                        if (!this->eat(CLexerTokenKind::RParen)) _ok = false;
                        return v;
                    }
                    default:
                        // Anything still named here was never defined.
                        if (is_name(t.kind)) return Value{0, false};
                        _ok = false;
                        return Value{};
                }
            }
            Value shift_right(Value a, std::uintmax_t n) const {
                if (a.is_unsigned) {
                    return Value{n >= IntWidth ? 0 : a.bits >> n, true};
                }
                const std::intmax_t s = static_cast<std::intmax_t>(a.bits);
                if (n >= IntWidth) {
                    return Value{static_cast<std::uintmax_t>(s < 0 ? -1 : 0),
                                 false};
                }
                return Value{static_cast<std::uintmax_t>(s >> n), false};
            }

            Value apply(CLexerTokenKind op, Value a, Value b) {
                const bool           u  = a.is_unsigned || b.is_unsigned;
                const std::uintmax_t x  = a.bits;
                const std::uintmax_t y  = b.bits;
                const std::intmax_t  sx = static_cast<std::intmax_t>(x);
                const std::intmax_t  sy = static_cast<std::intmax_t>(y);

                switch (op) {
                    case CLexerTokenKind::Pipe:
                        return Value{x | y, u};
                    case CLexerTokenKind::Caret:
                        return Value{x ^ y, u};
                    case CLexerTokenKind::Amp:
                        return Value{x & y, u};

                    case CLexerTokenKind::EqualEqual:
                        return make_bool(x == y);
                    case CLexerTokenKind::ExclamEqual:
                        return make_bool(x != y);
                    case CLexerTokenKind::Lesser:
                        return make_bool(u ? x < y : sx < sy);
                    case CLexerTokenKind::Greater:
                        return make_bool(u ? x > y : sx > sy);
                    case CLexerTokenKind::LesserEqual:
                        return make_bool(u ? x <= y : sx <= sy);
                    case CLexerTokenKind::GreaterEqual:
                        return make_bool(u ? x >= y : sx >= sy);

                    // A shift keeps the left operand's type; the right one
                    // only supplies a count.
                    case CLexerTokenKind::LesserLesser:
                        return Value{y >= IntWidth ? 0 : x << y, a.is_unsigned};
                    case CLexerTokenKind::GreaterGreater:
                        return this->shift_right(a, y);

                    case CLexerTokenKind::Plus:
                        return Value{x + y, u};
                    case CLexerTokenKind::Minus:
                        return Value{x - y, u};
                    case CLexerTokenKind::Asterisk:
                        return Value{x * y, u};

                    case CLexerTokenKind::Slash:
                    case CLexerTokenKind::Percent: {
                        const bool div = CLexerTokenKind::Slash == op;
                        if (0 == y) {
                            if (_live) _ok = false;
                            return Value{0, u};
                        }
                        if (u) return Value{div ? x / y : x % y, true};
                        if (-1 == sy &&
                            std::numeric_limits<std::intmax_t>::min() == sx) {
                            return Value{div ? x : 0, false};
                        }
                        return Value{static_cast<std::uintmax_t>(div ? sx / sy
                                                                     : sx % sy),
                                     false};
                    }

                    default:
                        _ok = false;
                        return Value{};
                }
            }
        };
    }  // namespace

    bool CPreprocessor::is_conditional(const std::string& keyword) {
        return "if" == keyword || keyword == "ifdef" || keyword == "ifndef" ||
               keyword == "elif" || keyword == "elifdef" ||
               keyword == "elifndef" || keyword == "else" || keyword == "endif";
    }

    std::vector<PpToken> CPreprocessor::apply_defined(
        const std::vector<PpToken>& tokens) const {
        std::vector<PpToken> out;
        out.reserve(tokens.size());

        for (std::size_t i = 0; i < tokens.size();) {
            if (CLexerTokenKind::Identifier != tokens[i].token.kind ||
                this->spelling(tokens[i].token) != "defined") {
                out.push_back(tokens[i]);
                ++i;
                continue;
            }
            std::size_t j     = i + 1;
            const bool  paren = j < tokens.size() &&
                                CLexerTokenKind::LParen == tokens[j].token.kind;
            if (paren) ++j;

            const bool named =
                j < tokens.size() && is_name(tokens[j].token.kind);
            const bool closed =
                !paren || (named && j + 1 < tokens.size() &&
                           CLexerTokenKind::RParen == tokens[j + 1].token.kind);

            if (!named || !closed) {
                // TODO: the parser should reject it.
                out.push_back(tokens[i]);
                ++i;
                continue;
            }
            const bool defined =
                _macros.contains(this->spelling(tokens[j].token));

            PpToken v = tokens[i];
            v.token.kind =
                defined ? CLexerTokenKind::True : CLexerTokenKind::False;
            out.push_back(v);

            i = paren ? j + 2 : j + 1;
        }
        return out;
    }
    bool CPreprocessor::eval_defined_name(const std::vector<PpToken>& tokens,
                                          const Directive& d, bool negate) {
        const std::size_t i = d.hash_index + 2;
        if (i >= d.end_index || !is_name(tokens[i].token.kind)) {
            _errors |= CPpErrorFlags::MalformedDirective;
            return false;
        }
        const bool defined = _macros.contains(this->spelling(tokens[i].token));
        return negate ? !defined : defined;
    }
    bool CPreprocessor::eval_condition(const std::vector<PpToken>& tokens,
                                       const Directive&            d) {
        if (d.hash_index + 2 >= d.end_index) {
            _errors |= CPpErrorFlags::MalformedDirective;
            return false;
        }

        std::vector<PpToken> line(tokens.begin() + d.hash_index + 2,
                                  tokens.begin() + d.end_index);

        constexpr TokenFlags strip =
            TokenFlags::DirectiveLine | TokenFlags::AtLineStart;
        for (auto& t: line) {
            t.token.flags &= static_cast<std::uint16_t>(~strip);
        }

        const std::vector<PpToken> named = this->apply_defined(line);

        std::vector<PpToken> expanded;
        for (std::size_t i = 0; i < named.size();) {
            i = this->expand_into(named, i, expanded);
        }

        std::vector<CondTerm> terms;
        terms.reserve(expanded.size());
        for (const auto& t: expanded) {
            terms.push_back(CondTerm{t.token.kind, this->spelling(t.token)});
        }

        bool       ok = true;
        CondEval   eval(terms);
        const bool value = eval.run(ok);
        if (!ok) _errors |= CPpErrorFlags::MalformedDirective;
        return value;
    }
    void CPreprocessor::handle_conditional(const std::vector<PpToken>& tokens,
                                           const Directive&            d) {
        const std::string& k = d.keyword;

        if ("if" == k || "ifdef" == k || "ifndef" == k) {
            if (this->skipping()) {
                _cond.push_back(CondLevel{false, true, false});
                return;
            }

            const bool v =
                k == "if" ? this->eval_condition(tokens, d)
                          : this->eval_defined_name(tokens, d, "ifndef" == k);
            _cond.push_back(CondLevel{v, v, false});
            return;
        }

        if (_cond.empty()) {
            _errors |= "endif" == k ? CPpErrorFlags::UnmatchedEndif
                                    : CPpErrorFlags::MalformedDirective;
            return;
        }

        if ("endif" == k) {
            _cond.pop_back();
            return;
        }

        CondLevel& top = _cond.back();
        if (top.in_else) {
            _errors |= CPpErrorFlags::MalformedDirective;
            return;
        }

        if (k == "else") {
            top.in_else = true;
            top.active  = !top.taken;
            top.taken   = true;
            return;
        }

        if (top.taken) {
            top.active = false;
            return;
        }

        const bool v =
            k == "elif" ? this->eval_condition(tokens, d)
                        : this->eval_defined_name(tokens, d, "elifndef" == k);
        top.active = v;
        top.taken  = v;
    }
}  // namespace Z::Zaban::Langs::CLang
