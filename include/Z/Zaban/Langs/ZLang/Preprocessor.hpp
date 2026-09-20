#pragma once

#include <Z/Zaban/PreProcess/PreprocessorBase.hpp>
#include <Z/Zaban/SourcePosition.hpp>
#include <cstdint>
#include <string>
#include <unordered_map>

#include "Z/Zaban/BitmaskEnum.hpp"
#include "Z/Zaban/Langs/ZLang/Lexer.hpp"

namespace Z::Zaban::Langs::ZLang {

    enum class ZPpErrorFlags : std::uint16_t {
        None               = 0,
        UnknownDirective   = 1 << 0,
        UnterminatedIf     = 1 << 1,
        UnmatchedElif      = 1 << 2,
        UnmatchedElse      = 1 << 3,
        UnmatchedEnd       = 1 << 4,
        ElseAfterElse      = 1 << 5,
        MalformedCondition = 1 << 6,
        UnKnownConfigKey   = 1 << 7,
    };

    /// One reported problem. 'code' holds exactly one bit of ZPpErrorFlags.
    struct ZPPDiagnostic {
        ZPpErrorFlags                   code;
        OffsetRange<ZLexerPositionType> range;
        /// Config variable name, or the offending spelling. Empty when there
        /// is nothing useful to show.
        std::string arg;
    };

    /* Where #if conditions get their values. Everything the manifest knows
       about (triplet fields, pointer width, project metadata) is supplied
       through this. The preprocessor never names a single one of those keys.
     */
    class ZConfigSource {
       public:
        virtual ~ZConfigSource() = default;

        /// True when the name is defined at all, whatever its value.
        virtual bool has(const std::string& name) const = 0;

        /// The name's value, or an empty string when it is not defined.
        virtual std::string get(const std::string& name) const = 0;
    };

    /// In memory config. allows testing as well as being used by callers
    /// without having a config map already.
    class MapConfigSource : public ZConfigSource {
       private:
        std::unordered_map<std::string, std::string> _values;

       public:
        MapConfigSource() = default;
        explicit MapConfigSource(
            std::unordered_map<std::string, std::string> values) :
            _values(std::move(values)) {
        }

        void define(std::string name, std::string value) {
            _values.insert_or_assign(std::move(name), std::move(value));
        }

        bool        has(const std::string& name) const override;
        std::string get(const std::string& name) const override;
    };

    /// shared empty config. every name is undefined!
    ZConfigSource& empty_config_source();

    enum class CondState : std::uint8_t {
        None = 0,
        /// tokens in the current branch are active
        ActiveTokens = 1 << 0,
        /// a branch was already taken. no later one can be taken!
        BranchTaken = 1 << 1,
        /// we're past '#else'. if we get to another '#elif' or '#else' its an
        /// error.
        InElse = 1 << 2,
    };

    /// One level of #if / #elif / #else / #end.
    struct CondLevel {
        CondState state;
        /// the range of '#' that opened this lvl
        OffsetRange<ZLexerPositionType> opened_at;
    };

    /// A directive in the stream. The hash and everything up to the next
    /// AtLineStart token.
    struct Directive {
        /// idx of the Hash token
        std::size_t hash_idx;
        /// one past the last token on the line
        std::size_t end_idx;
        /// 'if', 'elif', 'else', 'end'. 'if' lexes as a keyword but the rest
        /// are identifiers. so we check the string itself for now
        /// TODO: improve
        std::string keyword;
    };
}  // namespace Z::Zaban::Langs::ZLang

namespace Z::Zaban {
    Z_ENABLE_BITMASK_OPERATORS(Langs::ZLang::ZPpErrorFlags);
    Z_ENABLE_BITMASK_OPERATORS(Langs::ZLang::CondState);
}  // namespace Z::Zaban

namespace Z::Zaban::Langs::ZLang {
    class ZPreprocessor : public Pp::PreprocessorBase<ZLexerTokenType> {
       private:
        ZLexerBufferType   _source;
        ZLexerPositionType _base = 0;

        ZConfigSource*             _config = &empty_config_source();
        std::vector<CondLevel>     _cond;
        std::vector<ZPPDiagnostic> _diags;
        ZPpErrorFlags              _errors = ZPpErrorFlags::None;

        void report(ZPpErrorFlags code, OffsetRange<ZLexerPositionType> range,
                    std::string arg = {});

        /// True when we are inside a branch that is not being emitted
        bool skipping() const {
            return !_cond.empty() &&
                   !has(_cond.back().state, CondState::ActiveTokens);
        }

        /// true if t opens a directive/ has Hash flagged AtLineStart.
        bool is_directive_start(const ZLexerTokenType& t) const;

        /// reads the directive starting at 'i'. false if 'i' is not a directive
        /// start
        bool read_directive(const std::vector<ZLexerTokenType>& tokens,
                            std::size_t i, Directive& out) const;

        /// pushes, updates or pops _cond for one directive
        void handle_conditional(const std::vector<ZLexerTokenType>& tokens,
                                const Directive&                    d);

        /// parses and evals the condition in one pass.
        bool eval_condition(const std::vector<ZLexerTokenType>& tokens,
                            const Directive&                    d);

        /// main loop. marks. doesnt delete!
        void run(const std::vector<ZLexerTokenType>& in,
                 std::vector<ZLexerTokenType>&       out);

       public:
        explicit ZPreprocessor(ZLexerBufferType   source,
                               ZLexerPositionType base = 0) :
            _source(source), _base(base) {
        }

        std::vector<ZLexerTokenType> process(
            std::vector<ZLexerTokenType> tokens) override;

        ZPpErrorFlags errors() const {
            return _errors;
        }

        const std::vector<ZPPDiagnostic>& diagnostics() const {
            return _diags;
        }

        void set_config_source(ZConfigSource& source) {
            _config = &source;
        }

        std::string text_of(const ZLexerTokenType& t) const;
    };
}  // namespace Z::Zaban::Langs::ZLang
