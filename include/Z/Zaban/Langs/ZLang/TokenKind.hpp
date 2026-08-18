#pragma once

#include <ostream>

namespace Z::Zaban::Langs::ZLang {
    /** @brief Represents the different token categories produced by the lexer.
     *
     * TokenKind identifies the lexical category of each token encountered in
     * the source code. Tokens include operators, punctuation, literals,
     * identifiers, and language keywords.
     *
     * Values Dummy and Eof are reserved for internal lexer states.
     */
    enum class TokenKind {
        /** @brief Internal placeholder token. */
        Dummy = -3,

        /** @brief End-of-file marker. */
        Eof = -2,

        /** @brief End-of-buffer marker. */
        Eob = -1,

        /** @brief Arithmetic and bitwise operators. */
        Plus = 1,  // +
        Minus,     // -
        Asterisk,  // *
        Slash,     // /
        Percent,   // %
        Pipe,      // |
        Amp,       // &
        Equal,     // =
        Exclam,    // !
        Tilde,     // ~
        Caret,     // ^

        /** @brief Grouping and container delimiters. */
        LParen,  // (
        RParen,  // )
        LBrak,   // [
        RBrak,   // ]
        LBrace,  // {
        RBrace,  // }

        /** @brief Separators and member access tokens. */
        Dot,         // .
        DDot,        // ..
        Comma,       // ,
        Colon,       // :
        ColonColon,  // ::
        Semicolon,   // ;

        /** @brief Increment, logical, and custom operators. */
        PlusPlus,    // ++
        MinusMinus,  // --
        AmpAmp,      // &&
        AmpOp,       // &>
        PipePipe,    // ||

        /** @brief Comparison operators. */
        Lesser,          // <
        LesserEqual,     // <=
        LesserLesser,    // <<
        Greater,         // >
        GreaterEqual,    // >=
        GreaterGreater,  // >>
        EqualEqual,      // ==
        ExclamEqual,     // !=

        /** @brief Assignment operators. */
        PlusEqual,            // +=
        MinusEqual,           // -=
        AsteriskEqual,        // *=
        AsteriskOp,           // *>
        SlashEqual,           // /=
        PercentEqual,         // %=
        AmpEqual,             // &=
        PipeEqual,            // |=
        GreaterGreaterEqual,  // >>=
        LesserLesserEqual,    // <<=

        /** @brief Function, type, and declaration operators. */
        Arrow,     // ->
        EqualBig,  // =>

        /** @brief Conditional expression operators. */
        ColonEqual,  // :=
        Qmark,       // ?
        QAmp,        // ?&
        QPipe,       // ?|
        DQmark,      // ??
        QExclam,     // ?!
        DExclam,     // !!

        /** @brief Metadata operators. */
        AtSign,   // @
        DAtSign,  // @@
        AtColon,  // @:

        /** @brief Literal and identifier tokens. */
        Numeric,
        String,
        EndOfString,
        Identifier,

        /** @brief Language keywords. */
        Null,
        True,
        False,
        Let,
        Type,
        Return,
        Struct,
        Enum,
        If,
        EndIf,
        Loop,
        EndLoop,
        Func,
        Vari,
        Break,
        Continue,
        Goto,
        Label,
    };

    constexpr std::string_view to_string(TokenKind kind) {
        switch (kind) {
            case TokenKind::Dummy:
                return "Dummy";
            case TokenKind::Eof:
                return "Eof";
            case TokenKind::Eob:
                return "Eob";

            case TokenKind::Plus:
                return "Plus";
            case TokenKind::Minus:
                return "Minus";
            case TokenKind::Asterisk:
                return "Asterisk";
            case TokenKind::Slash:
                return "Slash";
            case TokenKind::Percent:
                return "Percent";
            case TokenKind::Pipe:
                return "Pipe";
            case TokenKind::Amp:
                return "Amp";
            case TokenKind::Equal:
                return "Equal";
            case TokenKind::Exclam:
                return "Exclam";
            case TokenKind::Tilde:
                return "Tilde";
            case TokenKind::Caret:
                return "Caret";

            case TokenKind::LParen:
                return "LParen";
            case TokenKind::RParen:
                return "RParen";
            case TokenKind::LBrak:
                return "LBrak";
            case TokenKind::RBrak:
                return "RBrak";
            case TokenKind::LBrace:
                return "LBrace";
            case TokenKind::RBrace:
                return "RBrace";

            case TokenKind::Dot:
                return "Dot";
            case TokenKind::DDot:
                return "DDot";
            case TokenKind::Comma:
                return "Comma";
            case TokenKind::Colon:
                return "Colon";
            case TokenKind::ColonColon:
                return "ColonColon";
            case TokenKind::Semicolon:
                return "Semicolon";

            case TokenKind::PlusPlus:
                return "PlusPlus";
            case TokenKind::MinusMinus:
                return "MinusMinus";
            case TokenKind::AmpAmp:
                return "AmpAmp";
            case TokenKind::AmpOp:
                return "AmpOp";
            case TokenKind::PipePipe:
                return "PipePipe";

            case TokenKind::Lesser:
                return "Lesser";
            case TokenKind::LesserEqual:
                return "LesserEqual";
            case TokenKind::LesserLesser:
                return "LesserLesser";
            case TokenKind::Greater:
                return "Greater";
            case TokenKind::GreaterEqual:
                return "GreaterEqual";
            case TokenKind::GreaterGreater:
                return "GreaterGreater";

            case TokenKind::PlusEqual:
                return "PlusEqual";
            case TokenKind::MinusEqual:
                return "MinusEqual";
            case TokenKind::AsteriskEqual:
                return "AsteriskEqual";
            case TokenKind::AsteriskOp:
                return "AsteriskOp";
            case TokenKind::SlashEqual:
                return "SlashEqual";
            case TokenKind::PercentEqual:
                return "PercentEqual";
            case TokenKind::AmpEqual:
                return "AmpEqual";
            case TokenKind::PipeEqual:
                return "PipeEqual";
            case TokenKind::EqualEqual:
                return "EqualEqual";
            case TokenKind::ExclamEqual:
                return "ExclamEqual";

            case TokenKind::GreaterGreaterEqual:
                return "GreaterGreaterEqual";
            case TokenKind::LesserLesserEqual:
                return "LesserLesserEqual";

            case TokenKind::Arrow:
                return "Arrow";
            case TokenKind::EqualBig:
                return "EqualBig";

            case TokenKind::ColonEqual:
                return "ColonEqual";
            case TokenKind::Qmark:
                return "Qmark";
            case TokenKind::QAmp:
                return "QAmp";
            case TokenKind::QPipe:
                return "QPipe";
            case TokenKind::DQmark:
                return "DQmark";
            case TokenKind::QExclam:
                return "QExclam";
            case TokenKind::DExclam:
                return "DExclam";

            case TokenKind::AtSign:
                return "AtSign";
            case TokenKind::DAtSign:
                return "DAtSign";
            case TokenKind::AtColon:
                return "AtColon";

            case TokenKind::Numeric:
                return "Numeric";
            case TokenKind::String:
                return "String";
            case TokenKind::EndOfString:
                return "EndOfString";
            case TokenKind::Identifier:
                return "Identifier";

            case TokenKind::Null:
                return "Null";
            case TokenKind::True:
                return "True";
            case TokenKind::False:
                return "False";
            case TokenKind::Let:
                return "Let";
            case TokenKind::Type:
                return "Type";
            case TokenKind::Return:
                return "Return";
            case TokenKind::Struct:
                return "Struct";
            case TokenKind::Enum:
                return "Enum";
            case TokenKind::If:
                return "If";
            case TokenKind::EndIf:
                return "EndIf";
            case TokenKind::Loop:
                return "Loop";
            case TokenKind::EndLoop:
                return "EndLoop";
            case TokenKind::Func:
                return "Func";
            case TokenKind::Vari:
                return "Vari";
            case TokenKind::Break:
                return "Break";
            case TokenKind::Continue:
                return "Continue";
            case TokenKind::Goto:
                return "Goto";
            case TokenKind::Label:
                return "Label";
        }

        return "<unknown>";
    }

    inline std::ostream& operator<<(std::ostream& os, TokenKind kind) {
        return os << to_string(kind);
    }
}  // namespace Z::Zaban::Langs::ZLang
