#pragma once

namespace Z::Zaban::ZLang {
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
        Dummy = -2,

        /** @brief End-of-file marker. */
        Eof = -1,

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

        /** @brief Assignment operators. */
        PlusEqual,      // +=
        MinusEqual,     // -=
        AsteriskEqual,  // *=
        AsteriskOp,     // *>
        SlashEqual,     // /=
        PercentEqual,   // %=
        AmpEqual,       // &=
        PipeEqual,      // |=
        EqualEqual,     // ==
        ExclamEqual,    // !=

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
}  // namespace Z::Zaban::ZLang
