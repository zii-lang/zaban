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

        /*
          explands t into out. by rescanning t he rplacement. 'active' holds
          the macro currently being expanded. a name in it is not re expanded
         */
        void expand_into(const CLexerTokenType&           t,
                         std::vector<CLexerTokenType>&    out,
                         std::unordered_set<std::string>& active) const;
    };
}  // namespace Z::Zaban::Langs::CLang
