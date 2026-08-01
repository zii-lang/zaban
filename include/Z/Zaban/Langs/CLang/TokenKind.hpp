#pragma once

namespace Z::Zaban::Langs::CLang {
    /** @brief Represents the different token categories produced by the lexer.
     *
     * TokenKind identifies the lexical category of each token encountered in
     * the source code. Tokens include operators, punctuation, literals,
     * identifiers, and language keywords.
     *
     * Keywords follow C23 (ISO/IEC 9899:2024). Alternate underscore-prefixed
     * spellings map to the same kind as their unprefixed counterpart, so
     * `_Bool` and `bool` both produce TokenKind::Bool.
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
        Ellipsis,    // ...
        Arrow,       // ->
        Comma,       // ,
        Colon,       // :
        ColonColon,  // :: (C23, attribute prefix)
        Semicolon,   // ;
        Question,    // ?

        /** @brief Increment, decrement, and logical operators. */
        PlusPlus,    // ++
        MinusMinus,  // --
        AmpAmp,      // &&
        PipePipe,    // ||

        /** @brief Comparison and shift operators. */
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
        SlashEqual,     // /=
        PercentEqual,   // %=
        AmpEqual,       // &=
        PipeEqual,      // |=
        CaretEqual,     // ^=
        EqualEqual,     // ==
        ExclamEqual,    // !=

        GreaterGreaterEqual,  // >>=
        LesserLesserEqual,    // <<=

        /** @brief Preprocessing operators. */
        Hash,      // #
        HashHash,  // ##

        /** @brief Literal and identifier tokens. */
        Numeric,
        String,
        CharLiteral,
        Identifier,

        /** @brief Language keywords. */
        Alignas,       // alignas, _Alignas
        Alignof,       // alignof, _Alignof
        Atomic,        // _Atomic
        Auto,          // auto
        BitInt,        // _BitInt
        Bool,          // bool, _Bool
        Break,         // break
        Case,          // case
        Char,          // char
        Complex,       // _Complex
        Const,         // const
        Constexpr,     // constexpr
        Continue,      // continue
        Decimal32,     // _Decimal32
        Decimal64,     // _Decimal64
        Decimal128,    // _Decimal128
        Default,       // default
        Do,            // do
        Double,        // double
        Else,          // else
        Enum,          // enum
        Extern,        // extern
        False,         // false
        Float,         // float
        For,           // for
        Generic,       // _Generic
        Goto,          // goto
        If,            // if
        Imaginary,     // _Imaginary
        Inline,        // inline
        Int,           // int
        Long,          // long
        Noreturn,      // _Noreturn
        Nullptr,       // nullptr
        Register,      // register
        Restrict,      // restrict
        Return,        // return
        Short,         // short
        Signed,        // signed
        Sizeof,        // sizeof
        Static,        // static
        StaticAssert,  // static_assert, _Static_assert
        Struct,        // struct
        Switch,        // switch
        ThreadLocal,   // thread_local, _Thread_local
        True,          // true
        Typedef,       // typedef
        Typeof,        // typeof
        TypeofUnqual,  // typeof_unqual
        Union,         // union
        Unsigned,      // unsigned
        Void,          // void
        Volatile,      // volatile
        While,         // while
    };
}  // namespace Z::Zaban::Langs::CLang
