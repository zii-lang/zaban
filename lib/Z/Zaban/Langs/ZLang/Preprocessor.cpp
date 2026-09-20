
#include <Z/Zaban/Langs/ZLang/Preprocessor.hpp>

#include "Z/Zaban/BitmaskEnum.hpp"
#include "Z/Zaban/Langs/ZLang/Lexer.hpp"
#include "Z/Zaban/SourcePosition.hpp"

namespace Z::Zaban::Langs::ZLang {
    namespace {
        std::string strip_quote(std::string s) {
            if (s.size() >= 2 && (s.front() == '"' || s.front() == '\'') &&
                s.back() == s.front()) {
                return s.substr(1, s.size() - 2);
            }
            return s;
        }

        /// parses and evals in one pass over the condition tokens
        /// doesnt need the AST
        class CondEval {
           public:
            CondEval(const ZPreprocessor& pp, const ZConfigSource& config,
                     const std::vector<ZLexerTokenType>& tokens,
                     std::size_t begin, std::size_t end) :
                _pp(pp), _config(config), _tokens(tokens), _pos(begin),
                _end(end) {
            }

            bool run() {
                if (this->at_end()) {
                    return this->fail();
                }

                const bool v = this->parse_or();

                // Trailing junk: '#if a b' must not parse as 'a'.
                if (_ok && !this->at_end()) {
                    return this->fail();
                }
                return v;
            }

            bool ok() const {
                return _ok;
            }

            const std::string& bad_key() const {
                return _bad_key;
            };

           private:
            const ZPreprocessor&                _pp;
            const ZConfigSource&                _config;
            const std::vector<ZLexerTokenType>& _tokens;
            std::size_t                         _pos;
            std::size_t                         _end;
            bool                                _ok = true;
            std::string                         _bad_key;

            static constexpr std::string_view CondKeys[] = {"vendor", "env",
                                                            "os", "arch"};

            bool check_key(const std::string& name) {
                if (std::find(std::begin(CondKeys), std::end(CondKeys), name) !=
                    std::end(CondKeys)) {
                    return true;
                }
                _bad_key = name;
                this->fail();
                return false;
            }

            bool at_end() const {
                return _pos >= _end;
            }

            /// Kind at _pos + n or Eof past the end of the condition
            ZLexerTokenKind peek(std::size_t n = 0) const {
                const std::size_t i = _pos + n;
                return i >= _end ? ZLexerTokenKind::Eof : _tokens[i].kind;
            }

            /// Marks the condition malformed and sets the cursor at the end
            /// so no loop above can spin on a token that wasnt consumed
            bool fail() {
                _ok  = false;
                _pos = _end;
                return false;
            }

            /// Only identifiers and strings have a string form, so only they
            /// can appear on either side of '==' or '!='.
            static bool is_operand(ZLexerTokenKind k) {
                return k == ZLexerTokenKind::Identifier ||
                       k == ZLexerTokenKind::String;
            }

            static bool is_cmp_op(ZLexerTokenKind k) {
                return k == ZLexerTokenKind::EqualEqual ||
                       k == ZLexerTokenKind::ExclamEqual;
            }

            /// String form of the operand at the cursor. Advances.
            std::string take_operand() {
                const auto& t = _tokens[_pos];
                ++_pos;

                if (t.kind == ZLexerTokenKind::String) {
                    return strip_quote(_pp.text_of(t));
                }
                auto name = _pp.text_of(t);
                if (!this->check_key(name)) return {};
                return _config.get(_pp.text_of(t));
            }

            /// or := and ( '||' and )*
            bool parse_or() {
                bool v = this->parse_and();
                while (_ok && this->peek() == ZLexerTokenKind::PipePipe) {
                    ++_pos;
                    const bool r = this->parse_and();
                    v            = v || r;
                }
                return v;
            }

            /// and := unary ( '&&' unary )*
            bool parse_and() {
                bool v = this->parse_unary();
                while (_ok && this->peek() == ZLexerTokenKind::AmpAmp) {
                    ++_pos;
                    const bool r = this->parse_unary();
                    v            = v && r;
                }
                return v;
            }

            /// unary := '!' unary | compare
            bool parse_unary() {
                if (this->peek() == ZLexerTokenKind::Exclam) {
                    ++_pos;
                    return !this->parse_unary();
                }
                return this->parse_compare();
            }

            /// compare := operand ( '==' | '!=' ) operand | primary
            ///
            /// Two tokens of lookahead pick the branch so primary can return
            /// a plain truth value instead of carrying a string form
            /// for a comparison that may never happen
            bool parse_compare() {
                if (!this->is_operand(this->peek()) ||
                    !this->is_cmp_op(this->peek(1))) {
                    return this->parse_primary();
                }

                const std::string l = this->take_operand();
                const auto        k = this->peek();
                ++_pos;

                if (!this->is_operand(this->peek())) {
                    return this->fail();
                }
                const std::string r = this->take_operand();
                if (!_ok) return false;

                return k == ZLexerTokenKind::EqualEqual ? l == r : l != r;
            }

            /// primary := '(' or ')' | string | identifier
            bool parse_primary() {
                if (this->at_end()) {
                    return this->fail();
                }

                const auto& t = _tokens[_pos];

                switch (t.kind) {
                    case ZLexerTokenKind::LParen: {
                        ++_pos;
                        const bool inner = this->parse_or();
                        if (!_ok) {
                            return false;
                        }
                        if (this->peek() != ZLexerTokenKind::RParen) {
                            return this->fail();
                        }
                        ++_pos;
                        return inner;
                    }

                    case ZLexerTokenKind::String:
                        ++_pos;
                        return !strip_quote(_pp.text_of(t)).empty();

                    case ZLexerTokenKind::Identifier: {
                        ++_pos;
                        auto name = _pp.text_of(t);
                        return this->check_key(name) && _config.has(name);
                    }
                    default:
                        return this->fail();
                }
            }
        };
    };  // namespace
    bool MapConfigSource::has(const std::string& name) const {
        return _values.contains(name);
    }

    std::string MapConfigSource::get(const std::string& name) const {
        const auto it = _values.find(name);
        return it == _values.end() ? std::string{} : it->second;
    }

    ZConfigSource& empty_config_source() {
        /// Every name undefined. Shared, so the default costs nothing.
        class EmptyConfig : public ZConfigSource {
           public:
            bool has(const std::string&) const override {
                return false;
            }

            std::string get(const std::string&) const override {
                return {};
            }
        };

        static EmptyConfig instance;
        return instance;
    }

    std::string ZPreprocessor::text_of(const ZLexerTokenType& t) const {
        if (t.range.begin > t.range.end || t.range.begin < _base) {
            return {};
        }
        const std::size_t from = t.range.begin - _base;
        const std::size_t len  = t.range.end - t.range.begin;
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
        while (j < tokens.size() && tokens[j].kind != ZLexerTokenKind::Eof &&
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
        const std::string& k = d.keyword;
        if (k == "if") {
            if (this->skipping()) {
                _cond.push_back(CondLevel{CondState::BranchTaken,
                                          tokens[d.hash_idx].range});
                return;
            }

            auto state = CondState::None;
            if (this->eval_condition(tokens, d)) {
                state |= CondState::ActiveTokens | CondState::BranchTaken;
            }
            _cond.push_back(CondLevel{state, tokens[d.hash_idx].range});
            return;
        }
        if (_cond.empty()) {
            report(k == "end"    ? ZPpErrorFlags::UnmatchedEnd
                   : k == "else" ? ZPpErrorFlags::UnmatchedElse
                                 : ZPpErrorFlags::UnmatchedElif,
                   tokens[d.hash_idx].range);
            return;
        }
        if (k == "end") {
            _cond.pop_back();
            return;
        }

        // if else is the last condlvl, then any cond after that is an err
        // the code shouldnt reach here if the conds were correctly written!
        auto& top = _cond.back();
        if (has(top.state, CondState::InElse)) {
            report(ZPpErrorFlags::ElseAfterElse, tokens[d.hash_idx].range,
                   d.keyword);
            return;
        }

        if (k == "else") {
            if (has(top.state, CondState::BranchTaken)) {
                top.state = unset(top.state, CondState::ActiveTokens);
            } else {
                top.state |= CondState::ActiveTokens;
            }
            top.state |= CondState::InElse | CondState::BranchTaken;
            return;
        }

        // what ever is left is 'elif'
        if (has(top.state, CondState::BranchTaken)) {
            top.state = unset(top.state, CondState::ActiveTokens);
            return;
        }

        if (this->eval_condition(tokens, d)) {
            top.state |= CondState::ActiveTokens | CondState::BranchTaken;
        } else {
            top.state = unset(top.state, CondState::ActiveTokens);
        }
    }

    bool ZPreprocessor::eval_condition(
        const std::vector<ZLexerTokenType>& tokens, const Directive& d) {
        const auto begin =
            d.hash_idx + 2 < d.end_idx ? d.hash_idx + 2 : d.end_idx;
        CondEval   eval(*this, *_config, tokens, begin, d.end_idx);
        const bool v = eval.run();
        if (!eval.ok()) {
            report(eval.bad_key().empty() ? ZPpErrorFlags::MalformedCondition
                                          : ZPpErrorFlags::UnKnownConfigKey,
                   tokens[d.hash_idx].range, eval.bad_key());
            return false;
        }
        return v;
    }

    void ZPreprocessor::run(const std::vector<ZLexerTokenType>& in,
                            std::vector<ZLexerTokenType>&       out) {
        out.reserve(in.size());

        std::size_t i = 0;
        while (i < in.size()) {
            Directive d;
            if (!this->read_directive(in, i, d)) {
                auto t = in[i];
                if (this->skipping() && t.kind != ZLexerTokenKind::Eof) {
                    // t.kind != ZLexerTokenKind::Eob) {
                    t.flags |= to_underlying(TokenFlags::Skipped);
                }
                out.push_back(t);
                ++i;
                continue;
            }

            const std::string& k = d.keyword;
            if (k == "if" || k == "elif" || k == "else" || k == "end") {
                this->handle_conditional(in, d);
            } else if (!this->skipping()) {
                report(ZPpErrorFlags::UnknownDirective, in[d.hash_idx].range,
                       k);
            }

            // the whole line
            for (std::size_t j = d.hash_idx; j < d.end_idx; ++j) {
                auto t = in[j];
                t.flags |= to_underlying(TokenFlags::DirectiveLine);
                out.push_back(t);
            }

            i = d.end_idx;
        }

        for (const auto& level: _cond) {
            report(ZPpErrorFlags::UnterminatedIf, level.opened_at);
        }
        _cond.clear();
    }

    std::vector<ZLexerTokenType> ZPreprocessor::process(
        std::vector<ZLexerTokenType> tokens) {
        std::vector<ZLexerTokenType> out;
        this->run(tokens, out);
        return out;
    }
}  // namespace Z::Zaban::Langs::ZLang
