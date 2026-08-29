#pragma once

#include <Z/Zaban/Langs/CLang/Lexer.hpp>
#include <Z/Zaban/PreProcess/PreprocessorBase.hpp>
#include <unordered_map>
#include <unordered_set>

namespace Z::Zaban::Langs::CLang {

    enum class CPpErrorFlags : std::uint8_t {
        None               = 0,
        UnknownDirective   = 1 << 0,
        UnterminatedIf     = 1 << 1,
        UnmatchedEndif     = 1 << 2,
        IncludeNotFound    = 1 << 3,
        MalformedDirective = 1 << 4,
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

    struct MacroDef {
        std::string                  name;
        std::vector<CLexerTokenType> body;
        bool                         function_like = false;
        std::vector<std::string>     params;
    };

    class CPreprocessor : public Pp::PreprocessorBase<CLexerTokenType> {
       public:
        explicit CPreprocessor(CLexerBufferType source) : _source(source) {
        }

        std::vector<CLexerTokenType> process(
            std::vector<CLexerTokenType> tokens) override;

       private:
        CLexerBufferType                          _source;
        std::unordered_map<std::string, MacroDef> _macros;

        /// True if t opens a directive like Hash at line start.
        bool is_directive_start(const CLexerTokenType& t) const;

        /// Spelling of t, unspliced when the token carries splice bytes.
        std::string spelling(const CLexerTokenType& t) const;

        /// Reads the directive beginning at `i`. Returns false if `i` is not
        /// a directive start.
        bool read_directive(const std::vector<CLexerTokenType>& tokens,
                            std::size_t i, Directive& out) const;

        void handle_define(const std::vector<CLexerTokenType>& tokens,
                           const Directive&                    d);
        void handle_undef(const std::vector<CLexerTokenType>& tokens,
                          const Directive&                    d);

        /* Index of the `(` opening an invocation of `tokens[i]`, or npos.
          The paren may sit on a later line. only the name and the paren
          matter, whitespace and newlines between them do not
         */
        std::size_t find_invocation_paren(
            const std::vector<CLexerTokenType>& tokens, std::size_t i) const;

        /* Splits the argument list starting at the `(`. On success `out`
          holds one token vector per argument and `end` is one past the `)`.
         */
        bool collect_arguments(const std::vector<CLexerTokenType>& tokens,
                               std::size_t                         lparen,
                               std::vector<std::vector<CLexerTokenType>>& out,
                               std::size_t& end) const;

        /// Substitutes `args` into `def.body`. TODO: no pre expansion. not yet
        std::vector<CLexerTokenType> substitute(
            const MacroDef&                                  def,
            const std::vector<std::vector<CLexerTokenType>>& args) const;

        /*
          explands t into out. by rescanning t he rplacement. 'active' holds
          the macro currently being expanded. a name in it is not re expanded
         */
        std::size_t expand_into(const std::vector<CLexerTokenType>& tokens,
                                std::size_t                         i,
                                std::vector<CLexerTokenType>&       out,
                                std::unordered_set<std::string>& active) const;
    };
}  // namespace Z::Zaban::Langs::CLang
