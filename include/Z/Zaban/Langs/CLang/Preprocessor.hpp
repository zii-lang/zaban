#pragma once

#include <Z/Zaban/Langs/CLang/Lexer.hpp>
#include <Z/Zaban/PreProcess/PreprocessorBase.hpp>

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

    class CPreprocessor : public Pp::PreprocessorBase<CLexerTokenType> {
       public:
        explicit CPreprocessor(CLexerBufferType source) : _source(source) {
        }

        std::vector<CLexerTokenType> process(
            std::vector<CLexerTokenType> tokens) override;

       private:
        CLexerBufferType _source;

        /// True if t opens a directive like Hash at line start.
        bool is_directive_start(const CLexerTokenType& t) const;

        /// Spelling of t, unspliced when the token carries splice bytes.
        std::string spelling(const CLexerTokenType& t) const;

        /// Reads the directive beginning at `i`. Returns false if `i` is not
        /// a directive start.
        bool read_directive(const std::vector<CLexerTokenType>& tokens,
                            std::size_t i, Directive& out) const;
    };
}  // namespace Z::Zaban::Langs::CLang
