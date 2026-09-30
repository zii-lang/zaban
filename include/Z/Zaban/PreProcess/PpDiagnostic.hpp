#pragma once

#include <Z/Zaban/Lex/LexerDiagnostic.hpp>
#include <Z/Zaban/SourcePosition.hpp>
#include <string_view>

namespace Z::Zaban::Pp {
    /// A language independent view of one preprocessor diagnostic. Carries
    /// what a renderer needs and none of the language's own code enum; cast
    /// the preprocessor down to reach that.
    struct PpDiagnosticView {
        Lex::LexerDiagnosticSeverity severity;
        OffsetRange<std::size_t>     range;
        /// Borrowed from the preprocessor that produced it.
        std::string_view arg;
    };
}  // namespace Z::Zaban::Pp
