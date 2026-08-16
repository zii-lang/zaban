#pragma once

#include <ostream>
#include <string_view>
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
        Dummy = -3,

        /** @brief End-of-file marker. */
        Eof = -2,

        /** @brief End-of-buffer marker (one chunk's end. dropped on concat). */
        Eob = -1,
        /** @brief Unterminated string fragment cut by chunk boundary*/
        StringOpen = -4,
        /** @brief Unterminated char fragment cut by chunk boundary*/
        CharOpen = -5,
        /** @brief Internal: two contiguous dots. Only ever a merge intermediate
         * on the way to Ellipsis. Never valid in finished C source. */
        DotDot = -6,
        /** @brief Unterminated block comment fragment cut by chunk boundary.
         *  Never reaches consumers: repair() drops it, finalize() drops it. */
        BlockCommentOpen = -7,
        /** @brief Unterminated line comment fragment cut by chunk boundary.
         *  Never reaches consumers: repair() drops it, finalize() drops it. */
        LineCommentOpen = -8,

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

    constexpr std::string_view to_string(TokenKind kind) {
        switch (kind) {
            case TokenKind::Dummy:
                return "Dummy";
            case TokenKind::Eof:
                return "Eof";
            case TokenKind::Eob:
                return "Eob";
            case TokenKind::StringOpen:
                return "StringOpen";
            case TokenKind::CharOpen:
                return "CharOpen";
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
            case TokenKind::DotDot:
                return "DotDot";
            case TokenKind::BlockCommentOpen:
                return "BlockCommentOpen";
            case TokenKind::LineCommentOpen:
                return "LineCommentOpen";
            case TokenKind::Ellipsis:
                return "Ellipsis";
            case TokenKind::Arrow:
                return "Arrow";
            case TokenKind::Comma:
                return "Comma";
            case TokenKind::Colon:
                return "Colon";
            case TokenKind::ColonColon:
                return "ColonColon";
            case TokenKind::Semicolon:
                return "Semicolon";
            case TokenKind::Question:
                return "Question";

            case TokenKind::PlusPlus:
                return "PlusPlus";
            case TokenKind::MinusMinus:
                return "MinusMinus";
            case TokenKind::AmpAmp:
                return "AmpAmp";
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
            case TokenKind::SlashEqual:
                return "SlashEqual";
            case TokenKind::PercentEqual:
                return "PercentEqual";
            case TokenKind::AmpEqual:
                return "AmpEqual";
            case TokenKind::PipeEqual:
                return "PipeEqual";
            case TokenKind::CaretEqual:
                return "CaretEqual";
            case TokenKind::EqualEqual:
                return "EqualEqual";
            case TokenKind::ExclamEqual:
                return "ExclamEqual";

            case TokenKind::GreaterGreaterEqual:
                return "GreaterGreaterEqual";
            case TokenKind::LesserLesserEqual:
                return "LesserLesserEqual";

            case TokenKind::Hash:
                return "Hash";
            case TokenKind::HashHash:
                return "HashHash";

            case TokenKind::Numeric:
                return "Numeric";
            case TokenKind::String:
                return "String";
            case TokenKind::CharLiteral:
                return "CharLiteral";
            case TokenKind::Identifier:
                return "Identifier";

            case TokenKind::Alignas:
                return "Alignas";
            case TokenKind::Alignof:
                return "Alignof";
            case TokenKind::Atomic:
                return "Atomic";
            case TokenKind::Auto:
                return "Auto";
            case TokenKind::BitInt:
                return "BitInt";
            case TokenKind::Bool:
                return "Bool";
            case TokenKind::Break:
                return "Break";
            case TokenKind::Case:
                return "Case";
            case TokenKind::Char:
                return "Char";
            case TokenKind::Complex:
                return "Complex";
            case TokenKind::Const:
                return "Const";
            case TokenKind::Constexpr:
                return "Constexpr";
            case TokenKind::Continue:
                return "Continue";
            case TokenKind::Decimal32:
                return "Decimal32";
            case TokenKind::Decimal64:
                return "Decimal64";
            case TokenKind::Decimal128:
                return "Decimal128";
            case TokenKind::Default:
                return "Default";
            case TokenKind::Do:
                return "Do";
            case TokenKind::Double:
                return "Double";
            case TokenKind::Else:
                return "Else";
            case TokenKind::Enum:
                return "Enum";
            case TokenKind::Extern:
                return "Extern";
            case TokenKind::False:
                return "False";
            case TokenKind::Float:
                return "Float";
            case TokenKind::For:
                return "For";
            case TokenKind::Generic:
                return "Generic";
            case TokenKind::Goto:
                return "Goto";
            case TokenKind::If:
                return "If";
            case TokenKind::Imaginary:
                return "Imaginary";
            case TokenKind::Inline:
                return "Inline";
            case TokenKind::Int:
                return "Int";
            case TokenKind::Long:
                return "Long";
            case TokenKind::Noreturn:
                return "Noreturn";
            case TokenKind::Nullptr:
                return "Nullptr";
            case TokenKind::Register:
                return "Register";
            case TokenKind::Restrict:
                return "Restrict";
            case TokenKind::Return:
                return "Return";
            case TokenKind::Short:
                return "Short";
            case TokenKind::Signed:
                return "Signed";
            case TokenKind::Sizeof:
                return "Sizeof";
            case TokenKind::Static:
                return "Static";
            case TokenKind::StaticAssert:
                return "StaticAssert";
            case TokenKind::Struct:
                return "Struct";
            case TokenKind::Switch:
                return "Switch";
            case TokenKind::ThreadLocal:
                return "ThreadLocal";
            case TokenKind::True:
                return "True";
            case TokenKind::Typedef:
                return "Typedef";
            case TokenKind::Typeof:
                return "Typeof";
            case TokenKind::TypeofUnqual:
                return "TypeofUnqual";
            case TokenKind::Union:
                return "Union";
            case TokenKind::Unsigned:
                return "Unsigned";
            case TokenKind::Void:
                return "Void";
            case TokenKind::Volatile:
                return "Volatile";
            case TokenKind::While:
                return "While";
        }
        return "<unknown>";
    }

    inline std::ostream& operator<<(std::ostream& os, TokenKind t) {
        return os << to_string(t);
    }
}  // namespace Z::Zaban::Langs::CLang
