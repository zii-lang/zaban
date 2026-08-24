#include <Z/Zaban/Langs/ZLang/Lexer.hpp>
#include <Z/Zaban/Langs/ZLang/TokenPair.hpp>
#include <optional>
#include <unordered_map>

namespace Z::Zaban::Langs::ZLang {
    static const std::unordered_map<TokenPair, ZLexerTokenKind, TokenPairHash>
        merges = {
            // ..
            {{ZLexerTokenKind::Dot, ZLexerTokenKind::Dot},
             ZLexerTokenKind::DDot},

            // ++
            {{ZLexerTokenKind::Plus, ZLexerTokenKind::Plus},
             ZLexerTokenKind::PlusPlus},

            // +=
            {{ZLexerTokenKind::Plus, ZLexerTokenKind::Equal},
             ZLexerTokenKind::PlusEqual},

            // ->
            {{ZLexerTokenKind::Minus, ZLexerTokenKind::Greater},
             ZLexerTokenKind::Arrow},

            // --
            {{ZLexerTokenKind::Minus, ZLexerTokenKind::Minus},
             ZLexerTokenKind::MinusMinus},

            // -=
            {{ZLexerTokenKind::Minus, ZLexerTokenKind::Equal},
             ZLexerTokenKind::MinusEqual},

            // *=
            {{ZLexerTokenKind::Asterisk, ZLexerTokenKind::Equal},
             ZLexerTokenKind::AsteriskEqual},

            // *>
            {{ZLexerTokenKind::Asterisk, ZLexerTokenKind::Greater},
             ZLexerTokenKind::AsteriskOp},

            // /=
            {{ZLexerTokenKind::Slash, ZLexerTokenKind::Equal},
             ZLexerTokenKind::SlashEqual},

            // %=
            {{ZLexerTokenKind::Percent, ZLexerTokenKind::Equal},
             ZLexerTokenKind::PercentEqual},

            // &=
            {{ZLexerTokenKind::Amp, ZLexerTokenKind::Equal},
             ZLexerTokenKind::AmpEqual},

            // &&
            {{ZLexerTokenKind::Amp, ZLexerTokenKind::Amp},
             ZLexerTokenKind::AmpAmp},

            // &>
            {{ZLexerTokenKind::Amp, ZLexerTokenKind::Greater},
             ZLexerTokenKind::AmpOp},

            // |=
            {{ZLexerTokenKind::Pipe, ZLexerTokenKind::Equal},
             ZLexerTokenKind::PipeEqual},

            // ||
            {{ZLexerTokenKind::Pipe, ZLexerTokenKind::Pipe},
             ZLexerTokenKind::PipePipe},

            // =>
            {{ZLexerTokenKind::Equal, ZLexerTokenKind::Greater},
             ZLexerTokenKind::EqualBig},

            // ==
            {{ZLexerTokenKind::Equal, ZLexerTokenKind::Equal},
             ZLexerTokenKind::EqualEqual},

            // !=
            {{ZLexerTokenKind::Exclam, ZLexerTokenKind::Equal},
             ZLexerTokenKind::ExclamEqual},

            // !!
            {{ZLexerTokenKind::Exclam, ZLexerTokenKind::Exclam},
             ZLexerTokenKind::DExclam},

            // <<
            {{ZLexerTokenKind::Lesser, ZLexerTokenKind::Lesser},
             ZLexerTokenKind::LesserLesser},

            // <=
            {{ZLexerTokenKind::Lesser, ZLexerTokenKind::Equal},
             ZLexerTokenKind::LesserEqual},

            // <<=
            {{ZLexerTokenKind::LesserLesser, ZLexerTokenKind::Equal},
             ZLexerTokenKind::LesserLesserEqual},

            // >>
            {{ZLexerTokenKind::Greater, ZLexerTokenKind::Greater},
             ZLexerTokenKind::GreaterGreater},

            // >=
            {{ZLexerTokenKind::Greater, ZLexerTokenKind::Equal},
             ZLexerTokenKind::GreaterEqual},

            // >>=
            {{ZLexerTokenKind::GreaterGreater, ZLexerTokenKind::Equal},
             ZLexerTokenKind::GreaterGreaterEqual},

            // @@
            {{ZLexerTokenKind::AtSign, ZLexerTokenKind::AtSign},
             ZLexerTokenKind::DAtSign},

            // @:
            {{ZLexerTokenKind::AtSign, ZLexerTokenKind::Colon},
             ZLexerTokenKind::AtColon},

            // ::
            {{ZLexerTokenKind::Colon, ZLexerTokenKind::Colon},
             ZLexerTokenKind::ColonColon},

            // :=
            {{ZLexerTokenKind::Colon, ZLexerTokenKind::Equal},
             ZLexerTokenKind::ColonEqual},

            // ??
            {{ZLexerTokenKind::Qmark, ZLexerTokenKind::Qmark},
             ZLexerTokenKind::DQmark},

            // ?!
            {{ZLexerTokenKind::Qmark, ZLexerTokenKind::Exclam},
             ZLexerTokenKind::QExclam},

            // ?&
            {{ZLexerTokenKind::Qmark, ZLexerTokenKind::Amp},
             ZLexerTokenKind::QAmp},

            // ?|
            {{ZLexerTokenKind::Qmark, ZLexerTokenKind::Pipe},
             ZLexerTokenKind::QPipe},

            {{ZLexerTokenKind::String, ZLexerTokenKind::EndOfString},
             ZLexerTokenKind::String},

            {{ZLexerTokenKind::Numeric, ZLexerTokenKind::Numeric},
             ZLexerTokenKind::Numeric},
    };

    static std::optional<ZLexerTokenKind> merge(ZLexerTokenKind lhs,
                                                ZLexerTokenKind rhs) {
        auto it = merges.find({lhs, rhs});

        if (it == merges.end()) {
            return std::nullopt;
        }

        return it->second;
    }

    static void merge_double_tokens(ZLexer& lexer) {
        std::vector<ZLexerTokenType> merged_tokens;

        const auto base_token_size = lexer.get_tokens().size();
        merged_tokens.reserve(base_token_size);

        for (std::size_t i = 0; i < base_token_size;) {
            auto& current = lexer.get_token(i);

            if (current.kind == ZLexerTokenKind::Eob ||
                current.kind == ZLexerTokenKind::Eof) {
                ++i;
                continue;
            }

            auto token = std::move(current);
            ++i;

            while (i < base_token_size) {
                auto& next = lexer.get_token(i);

                if (next.kind == ZLexerTokenKind::Eob ||
                    next.kind == ZLexerTokenKind::Eof) {
                    break;
                }

                if (token.kind != ZLexerTokenKind::String &&
                    token.range.end + 1 != next.range.begin) {
                    break;
                }

                auto merge_kind = merge(token.kind, next.kind);

                if (!merge_kind) {
                    break;
                }

                token.kind      = *merge_kind;
                token.range.end = next.range.end;

                ++i;
            }

            merged_tokens.push_back(std::move(token));
        }

        if (merged_tokens.empty()) {
            const auto offset = lexer.get_offset();

            merged_tokens.emplace_back(
                ZLexerTokenKind::Eof,
                OffsetRange<ZLexerPositionType>(offset, offset));
        } else {
            const auto eof_pos = merged_tokens.back().range.end + 1;

            merged_tokens.emplace_back(
                ZLexerTokenKind::Eof,
                OffsetRange<ZLexerPositionType>(eof_pos, eof_pos));
        }

        lexer.set_tokens(std::move(merged_tokens));
    }

    static void merge_identifier_boundary(ZLexer& lhs, ZLexer& rhs) {
        auto find_last_real = [](auto& tokens) {
            for (std::size_t i = tokens.size(); i > 0; --i) {
                const auto index = i - 1;

                if (tokens[index].kind != ZLexerTokenKind::Eob &&
                    tokens[index].kind != ZLexerTokenKind::Eof) {
                    return index;
                }
            }

            return tokens.size();
        };

        auto find_first_real = [](auto& tokens) {
            for (std::size_t i = 0; i < tokens.size(); ++i) {
                if (tokens[i].kind != ZLexerTokenKind::Eob &&
                    tokens[i].kind != ZLexerTokenKind::Eof) {
                    return i;
                }
            }

            return tokens.size();
        };

        const auto lhs_index = find_last_real(lhs.get_tokens());
        const auto rhs_index = find_first_real(rhs.get_tokens());

        if (lhs_index == lhs.get_tokens().size() ||
            rhs_index == rhs.get_tokens().size()) {
            return;
        }

        auto& lhs_token = lhs.get_token(lhs_index);
        auto& rhs_token = rhs.get_token(rhs_index);

        if (lhs_token.kind != ZLexerTokenKind::Identifier ||
            rhs_token.kind != ZLexerTokenKind::Identifier) {
            return;
        }

        if (lhs_token.range.end + 1 != rhs_token.range.begin) {
            return;
        }

        const auto lhs_text = token_text(lhs, lhs_token);
        const auto rhs_text = token_text(rhs, rhs_token);

        const auto combined = lhs_text + rhs_text;

        lhs_token.kind      = classify_identifier(combined);
        lhs_token.range.end = rhs_token.range.end;

        rhs.get_tokens().erase(rhs.get_tokens().begin() +
                               static_cast<std::ptrdiff_t>(rhs_index));
    };

    void ZLexer::merge() {
        merge_double_tokens(*this);
        this->_flags = unset(this->_flags, ZLexerInvalidationFlag::NeedsMerge);
    }

    void ZLexer::merge(ZLexer& rhs) {
        merge_identifier_boundary(*this, rhs);
        merge_double_tokens(*this);
        this->_flags = unset(this->_flags, ZLexerInvalidationFlag::NeedsMerge);
    }
}  // namespace Z::Zaban::Langs::ZLang
