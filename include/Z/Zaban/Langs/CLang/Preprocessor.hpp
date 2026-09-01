#pragma once

#include <Z/Zaban/Langs/CLang/Lexer.hpp>
#include <Z/Zaban/PreProcess/HideSet.hpp>
#include <Z/Zaban/PreProcess/PreprocessorBase.hpp>
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
    };

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

    struct MacroDef {
        std::string              name;
        std::vector<PpToken>     body;
        bool                     function_like = false;
        std::vector<std::string> params;
    };

    class CPreprocessor : public Pp::PreprocessorBase<CLexerTokenType> {
       public:
        explicit CPreprocessor(CLexerBufferType source) : _source(source) {
        }

        std::vector<CLexerTokenType> process(
            std::vector<CLexerTokenType> tokens) override;
        CPpErrorFlags errors() const {
            return _errors;
        }

       private:
        CLexerBufferType                          _source;
        std::unordered_map<std::string, MacroDef> _macros;
        Pp::HideSetTable                          _hide_sets;
        std::vector<CondLevel>                    _cond;
        CPpErrorFlags                             _errors = CPpErrorFlags::None;

        /// True if t opens a directive like Hash at line start.
        bool is_directive_start(const CLexerTokenType& t) const;

        /// Spelling of t, unspliced when the token carries splice bytes.
        std::string spelling(const CLexerTokenType& t) const;

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
        bool collect_arguments(const std::vector<PpToken>&        tokens,
                               std::size_t                        lparen,
                               std::vector<std::vector<PpToken>>& out,
                               std::size_t&                       end) const;

        /// Substitutes `args` into `def.body`.
        std::vector<PpToken> substitute(
            const MacroDef&                          def,
            const std::vector<std::vector<PpToken>>& args) const;

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
    };
}  // namespace Z::Zaban::Langs::CLang

namespace Z::Zaban {
    Z_ENABLE_BITMASK_OPERATORS(Langs::CLang::CPpErrorFlags);
}
