#pragma once

#include <Z/Zaban/Langs/CLang/Lexer.hpp>
#include <Z/Zaban/PreProcess/HideSet.hpp>
#include <Z/Zaban/PreProcess/PreprocessorBase.hpp>
#include <deque>
#include <unordered_map>

#include "Z/Zaban/BitmaskEnum.hpp"

namespace Z::Zaban::Langs::CLang {

    enum class CPpErrorFlags : std::uint8_t {
        None               = 0,
        UnknownDirective   = 1 << 0,
        UnterminatedIf     = 1 << 1,
        UnmatchedEndif     = 1 << 2,
        IncludeNotFound    = 1 << 3,
        MalformedDirective = 1 << 4,
        InvalidPaste       = 1 << 5,
        IncludeTooDeep     = 1 << 6,
    };

    /* Where a token's spelling lives. the main source, one entry per
          included file, one per synthesized token. Buffers sit end to end in a
          single offset space, so a token range identifies both the buffer and
          the position inside it and Token needs no new field.  so an included
       file is lexed with its base as start_pos and its ranges come out
       absolute.
        */
    class PpSources {
       public:
        PpSources(CLexerBufferType main, std::string name) {
            _entries.push_back(Entry{{}, main, std::move(name), 0});
            _next = main.size() + 1;
        }

        /// Registers owned text and returns the base offset it now lives at.
        std::size_t add(std::string text, std::string name) {
            const std::size_t base = _next;
            _next += text.size() + 1;

            _entries.push_back(
                Entry{std::move(text), {}, std::move(name), base});

            // The deque keeps the string put, so the view stays valid.
            Entry& e = _entries.back();
            e.text   = e.owned;
            return base;
        }

        CLexerBufferType whole(std::size_t base) const {
            return this->find(base).text;
        }

        CLexerBufferType text(std::size_t begin, std::size_t end) const {
            const Entry&      e    = this->find(begin);
            const std::size_t from = begin - e.base;
            const std::size_t len  = end - begin;
            return from + len <= e.text.size() ? e.text.substr(from, len)
                                               : CLexerBufferType{};
        }

        const std::string& name(std::size_t offset) const {
            return this->find(offset).name;
        }

       private:
        struct Entry {
            std::string      owned;
            CLexerBufferType text;
            std::string      name;
            std::size_t      base;
        };

        /// Bases increase, so the owner is the last entry at or below offset.
        const Entry& find(std::size_t offset) const {
            std::size_t lo = 0;
            std::size_t hi = _entries.size();
            while (hi - lo > 1) {
                const std::size_t mid = lo + (hi - lo) / 2;
                if (_entries[mid].base <= offset) {
                    lo = mid;
                } else {
                    hi = mid;
                }
            }
            return _entries[lo];
        }

        std::deque<Entry> _entries;
        std::size_t       _next = 0;
    };

    /* Where #include gets its bytes. The default reads from disk. The LSP
           has unsaved buffers
         */
    class IncludeSource {
       public:
        virtual ~IncludeSource() = default;

        /// Resolved path to contents. False if there is nothing there.
        virtual bool read(const std::string& path, std::string& out) = 0;
    };

    IncludeSource& disk_include_source();

    /// One Level of the #if/#elif/#else/#endif
    struct CondLevel {
        /// tokens in the current branch are active
        bool active;
        /// a branch has already been taken. no later one can be taken
        bool taken;
        /// #else. another #elif or #else is an err
        bool in_else;
    };

    /// A directive found in the stream: the Hash and everything up to the
    /// next AtLineStart token.
    struct Directive {
        // index of the Hash token
        std::size_t hash_index;
        // one past the last token on the line
        std::size_t end_index;
        // Identifier, If, Else, ...
        CLexerTokenKind keyword_kind;
        // "define", "include", "if", ...
        std::string keyword;
    };

    /// A token plus the macros already expanded to produce it.
    struct PpToken {
        CLexerTokenType token;
        Pp::HideSetId   hides = Pp::HideSetTable::Empty;
    };

    /* One macro argument in both forms. # and ## take the unexpanded one,
          every other occurrence takes the expanded one, and which is which is
          decided per occurrence in the body, not per argument.
        */
    struct MacroArg {
        std::vector<PpToken> raw;
        std::vector<PpToken> expanded;
    };

    struct MacroDef {
        std::string              name;
        std::vector<PpToken>     body;
        bool                     function_like = false;
        std::vector<std::string> params;
    };

    class CPreprocessor : public Pp::PreprocessorBase<CLexerTokenType> {
       public:
        explicit CPreprocessor(CLexerBufferType source,
                               std::string      name = "<soruce>") :
            _sources(source, std::move(name)) {
        }

        std::vector<CLexerTokenType> process(
            std::vector<CLexerTokenType> tokens) override;
        CPpErrorFlags errors() const {
            return _errors;
        }
        void add_include_dir(std::string dir) {
            _include_dirs.push_back(std::move(dir));
        }

        void set_include_source(IncludeSource& source) {
            _reader = &source;
        }

        /// Spelling of t, unspliced when the token carries splice bytes.
        std::string spelling(const CLexerTokenType& t) const;

       private:
        static constexpr std::size_t MaxIncludeDepth = 200;

        PpSources                                 _sources;
        std::unordered_map<std::string, MacroDef> _macros;
        Pp::HideSetTable                          _hide_sets;
        std::vector<CondLevel>                    _cond;
        CPpErrorFlags                             _errors = CPpErrorFlags::None;
        /// The include stack, innermost last. relative resolution
        std::vector<std::string> _files;
        std::vector<std::string> _include_dirs;
        IncludeSource*           _reader = &disk_include_source();

        /// True if t opens a directive like Hash at line start.
        bool is_directive_start(const CLexerTokenType& t) const;

        /// Reads the directive beginning at `i`. Returns false if `i` is not
        /// a directive start.
        bool read_directive(const std::vector<PpToken>& tokens, std::size_t i,
                            Directive& out) const;

        void handle_define(const std::vector<PpToken>& tokens,
                           const Directive&            d);
        void handle_undef(const std::vector<PpToken>& tokens,
                          const Directive&            d);

        /* Index of the `(` opening an invocation of `tokens[i]`, or npos.
          The paren may sit on a later line. only the name and the paren
          matter, whitespace and newlines between them do not
         */
        std::size_t find_invocation_paren(const std::vector<PpToken>& tokens,
                                          std::size_t                 i) const;

        /* Splits the argument list starting at the `(`. On success `out`
          holds one token vector per argument and `end` is one past the `)`.
         */
        bool collect_arguments(const std::vector<PpToken>& tokens,
                               std::size_t lparen, std::vector<MacroArg>& out,
                               std::size_t& end) const;

        /// Substitutes `args` into `def.body`.
        std::vector<PpToken> substitute(const MacroDef&              def,
                                        const std::vector<MacroArg>& args);

        /*
         Expands tokens[i] into out, rescanning the replacement. A macro
         whose name is in the token's hide set is not expanded again.
         */
        std::size_t expand_into(const std::vector<PpToken>& tokens,
                                std::size_t i, std::vector<PpToken>& out);
        bool        skipping() const {
            return !_cond.empty() && !_cond.back().active;
        }

        static bool is_conditional(const std::string& keyword);
        void        handle_conditional(const std::vector<PpToken>& tokens,
                                       const Directive&            d);
        /// #ifdef/#ifndef/#elifdef/#elifndef
        bool eval_defined_name(const std::vector<PpToken>& tokens,
                               const Directive& d, bool negate);
        /// #if/#elif
        bool eval_condition(const std::vector<PpToken>& tokens,
                            const Directive&            d);
        /// replaces every 'defined X' and 'defined(X)' with true or false
        /// its done before macro expansion happens on that line
        std::vector<PpToken> apply_defined(
            const std::vector<PpToken>& tokens) const;
        /// Index of the parameter body[i] names, or npos.
        std::size_t param_index(const MacroDef&             def,
                                const std::vector<PpToken>& body,
                                std::size_t                 i) const;

        /// `#arg`: the argument's spelling as a string literal
        PpToken stringize(const std::vector<PpToken>& arg, const PpToken& at);

        /// `a##b`: concatenate the spellings and re-lex. False if the
        /// result is not exactly one token
        bool paste(const PpToken& a, const PpToken& b, PpToken& out);

        void handle_include(std::vector<PpToken>& tokens, const Directive& d,
                            std::vector<PpToken>& out);

        /// Pulls the header name out of the line. `"x.h"` is one String
        /// token; `<x.h>` is a bracketed run that has to be read back from
        /// the buffer.
        bool read_header_name(const std::vector<PpToken>& line,
                              std::string& path, bool& angled) const;

        bool find_header(const std::string& name, bool angled,
                         std::string& resolved, std::string& text) const;
        /// The main loop. #include reenters it with the header's tokens
        void run(std::vector<PpToken>& in, std::vector<PpToken>& out);
    };
}  // namespace Z::Zaban::Langs::CLang

namespace Z::Zaban {
    Z_ENABLE_BITMASK_OPERATORS(Langs::CLang::CPpErrorFlags);
}
