#include <Z/Zaban/Config.hpp>
#include <Z/Zaban/Langs/ZLang/Lexer.hpp>
#include <Z/Zaban/Lex/CharUtil.hpp>
#include <Z/Zaban/Lex/ScanUtil.hpp>
#include <array>
#include <cstddef>
#include <iterator>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Z::Zaban::Langs::ZLang {
    static const std::unordered_map<std::string, ZLexerTokenKind>
        ZLangKeywords = {
            {"null", ZLexerTokenKind::Null},
            {"true", ZLexerTokenKind::True},
            {"false", ZLexerTokenKind::False},
            {"let", ZLexerTokenKind::Let},
            {"type", ZLexerTokenKind::Type},
            {"return", ZLexerTokenKind::Return},
            {"struct", ZLexerTokenKind::Struct},
            {"enum", ZLexerTokenKind::Enum},
            {"if", ZLexerTokenKind::If},
            {"endif", ZLexerTokenKind::EndIf},
            {"loop", ZLexerTokenKind::Loop},
            {"endloop", ZLexerTokenKind::EndLoop},
            {"func", ZLexerTokenKind::Func},
            {"vari", ZLexerTokenKind::Vari},
            {"break", ZLexerTokenKind::Break},
            {"continue", ZLexerTokenKind::Continue},
            {"goto", ZLexerTokenKind::Goto},
            {"label", ZLexerTokenKind::Label},
    };

    struct TokenPair {
        ZLexerTokenKind lhs;
        ZLexerTokenKind rhs;

        bool operator==(const TokenPair&) const = default;
    };

    struct TokenPairHash {
        std::size_t operator()(const TokenPair& pair) const noexcept {
            const auto lhs = static_cast<std::size_t>(pair.lhs);
            const auto rhs = static_cast<std::size_t>(pair.rhs);

            // Avoid assuming that std::size_t is 64-bit.
            constexpr std::size_t offset = sizeof(std::size_t) * 8 / 2;
            return (lhs << offset) ^ rhs;
        }
    };

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

    ZLexer::ZLexer(ZLexerBufferType& buffer) :
        Zaban::Lex::Lexer<ZLexerTokenType, ZLexerPositionType,
                          ZLexerBufferType>(buffer),
        _buffer_it(buffer.begin()) {};

    ZLexer::ZLexer(ZLexerBufferType& buffer, ZLexerPositionType start_pos) :
        Zaban::Lex::Lexer<ZLexerTokenType, ZLexerPositionType,
                          ZLexerBufferType>(buffer, start_pos),
        _buffer_it(buffer.begin()) {};

    void ZLexer::set_buffer(ZLexerBufferType& buffer) {
        this->_buffer    = buffer;
        this->_buffer_it = this->_buffer.begin();
    }

    ZLexerPositionType ZLexer::get_offset() {
        return this->_offset;
    }

    void ZLexer::set_offset(ZLexerPositionType offset) {
        this->_offset = offset;
    }

    ZLexerBufferType::const_pointer ZLexer::peek() const {
        return this->peek(0);
    }

    ZLexerBufferType::const_pointer ZLexer::peek(
        const ZLexerPositionType offset) const {
        if (this->_buffer_it + offset >= this->_buffer.end()) {
            return nullptr;
        }
        return std::to_address(this->_buffer_it + offset);
    }

    void ZLexer::advance() {
        this->advance(1);
    }

    void ZLexer::advance(const ZLexerPositionType offset) {
        if (0 == offset || (this->_buffer_it + offset > this->_buffer.end())) {
            return;
        }
        this->_buffer_it += offset;
        this->_offset += offset;
    }

    void ZLexer::set_lexer_state(const ZLexerInternalState state) {
        this->_state = state;
    }

    bool ZLexer::scan_until(ZLexerBufferType::value_type ch) {
        ZLexerBufferType::const_pointer p0 = this->peek();
        if (nullptr == p0) {
            return false;
        }

        for (; this->_buffer_it != this->_buffer.end(); this->advance()) {
            p0 = this->peek();
            if (ch == *p0) {
                return true;
            }
        }

        return false;
    }

    bool ZLexer::scan_newline() {
        ZLexerBufferType::const_pointer p0 = this->peek();
        if (nullptr == p0 || !Lex::CharUtil::is_linefeed(*p0)) {
            return false;
        }

        // true from here
        ZLexerBufferType::const_pointer p1 = this->peek(1);
        if (nullptr == p1) {
            this->advance();
            return true;
        }

        ZLexerPositionType line_char_count = 0;
        Lex::ScanUtil::is_newline_seq(*p0, *p1, &line_char_count);
        this->advance(line_char_count);
        return true;
    }

    bool ZLexer::scan_until_newline() {
        for (; this->_buffer_it != this->_buffer.end(); this->advance()) {
            if (this->scan_newline()) {
                return true;
            }
        }
        return false;
    }

    bool ZLexer::scan_comment() {
        ZLexerBufferType::const_pointer p0 = this->peek();
        if (nullptr == p0 || '/' != *p0) {
            return false;
        }

        ZLexerBufferType::const_pointer p1 = this->peek(1);
        if (nullptr == p1 ||
            !Zaban::Lex::ScanUtil::is_either_slash_comment(*p0, *p1)) {
            return false;
        }

        this->advance(2);

        if (Zaban::Lex::ScanUtil::is_double_slash_comment(*p0, *p1)) {
            this->set_lexer_state(ZLexerInternalState::LineComment);
            return this->scan_double_slash_close_comment();
        } else {
            this->set_lexer_state(ZLexerInternalState::BlockComment);
            return this->scan_until_block_slash_close_comment();
        }
    }

    bool ZLexer::scan_double_slash_close_comment() {
        if (this->scan_until_newline()) {
            this->set_lexer_state(ZLexerInternalState::Normal);
            return true;
        }
        return false;
    }

    bool ZLexer::scan_until_block_slash_close_comment() {
        for (; this->_buffer_it != this->_buffer.end(); this->advance()) {
            ZLexerBufferType::const_pointer p0 = this->peek();
            ZLexerBufferType::const_pointer p1 = this->peek(1);

            if (nullptr == p0 || nullptr == p1) {
                return false;
            }

            if (Zaban::Lex::ScanUtil::is_block_slash_comment_end(*p0, *p1)) {
                this->advance(2);
                this->set_lexer_state(ZLexerInternalState::Normal);
                return true;
            }
        }

        return false;
    }

    bool ZLexer::scan_until_eos() {
        if (this->_state != ZLexerInternalState::SQString &&
            this->_state != ZLexerInternalState::DQString)
            Z_UNLIKELY {
                return true;
            }

        ZLexerBufferType::value_type p0 =
            this->_state == ZLexerInternalState::SQString ? '\'' : '"';
        if (scan_until(p0)) {
            this->_tokens.emplace_back(
                ZLexerTokenKind::EndOfString,
                SourcePositionRange<ZLexerPositionType>(_offset, _offset));
            this->set_lexer_state(ZLexerInternalState::Normal);
            this->advance();
            return true;
        } else
            Z_UNLIKELY {
                return false;
            }
    }

    bool ZLexer::scan_until_get_numeric() {
        auto set_numeric_error = [this]() {
            this->_state               = ZLexerInternalState::Error;
            this->_diagnostics._errors = set(this->_diagnostics._errors,
                                             ZLexerErrorFlag::InvalidCharacter);
        };

        auto consume_digits = [this](auto predicate) {
            bool consumed = false;

            while (const auto* p = this->peek()) {
                if (!predicate(*p)) {
                    break;
                }

                consumed = true;
                this->advance();
            }

            return consumed;
        };

        for (;;) {
            switch (this->_state) {
                case ZLexerInternalState::ZeroStart: {
                    const auto* p = this->peek();

                    // Keep ZeroStart alive across a chunk boundary.
                    // The next chunk may turn `0` into `0x`, `0.`, `0e`,
                    // or continue the decimal digits.
                    if (p == nullptr) {
                        return true;
                    }

                    switch (*p) {
                        case 'x':
                        case 'X':
                            this->advance();
                            this->_state = ZLexerInternalState::HexNumber;
                            continue;

                        case 'o':
                        case 'O':
                            this->advance();
                            this->_state = ZLexerInternalState::OctNumber;
                            continue;

                        case 'b':
                        case 'B':
                            this->advance();
                            this->_state = ZLexerInternalState::BinNumber;
                            continue;

                        case '.':
                            this->advance();
                            this->_state = ZLexerInternalState::FloatNumber;
                            continue;

                        case 'e':
                        case 'E':
                            this->advance();

                            if (const auto* sign = this->peek();
                                sign != nullptr &&
                                (*sign == '+' || *sign == '-')) {
                                this->advance();
                            }

                            this->_state =
                                ZLexerInternalState::ScientificNumber;
                            continue;

                        default:
                            if (Zaban::Lex::CharUtil::is_digit(*p)) {
                                this->advance();
                                this->_state = ZLexerInternalState::Number;
                                continue;
                            }

                            this->_state = ZLexerInternalState::Normal;
                            return true;
                    }
                }

                case ZLexerInternalState::Number: {
                    const auto* p = this->peek();

                    // IMPORTANT:
                    // Do not transition to Normal here. `100` may be
                    // continued by `.10`, `E10`, `e+10`, etc. in the next
                    // input chunk.
                    if (p == nullptr) {
                        return true;
                    }

                    if (Zaban::Lex::CharUtil::is_digit(*p)) {
                        this->advance();
                        continue;
                    }

                    if (*p == '.') {
                        this->advance();
                        this->_state = ZLexerInternalState::FloatNumber;
                        continue;
                    }

                    if (*p == 'e' || *p == 'E') {
                        this->advance();

                        if (const auto* sign = this->peek();
                            sign != nullptr && (*sign == '+' || *sign == '-')) {
                            this->advance();
                        }

                        this->_state = ZLexerInternalState::ScientificNumber;
                        continue;
                    }

                    this->_state = ZLexerInternalState::Normal;
                    return true;
                }

                case ZLexerInternalState::FloatNumber: {
                    const auto* p = this->peek();

                    // A float may still be continued by an exponent in the
                    // next chunk.
                    if (p == nullptr) {
                        return true;
                    }

                    if (Zaban::Lex::CharUtil::is_digit(*p)) {
                        this->advance();
                        continue;
                    }

                    if (*p == 'e' || *p == 'E') {
                        this->advance();

                        if (const auto* sign = this->peek();
                            sign != nullptr && (*sign == '+' || *sign == '-')) {
                            this->advance();
                        }

                        this->_state = ZLexerInternalState::ScientificNumber;
                        continue;
                    }

                    this->_state = ZLexerInternalState::Normal;
                    return true;
                }

                case ZLexerInternalState::ScientificNumber: {
                    const bool consumed =
                        consume_digits(Zaban::Lex::CharUtil::is_digit);

                    if (!consumed) {
                        // If we are at EOB, the exponent is incomplete and
                        // may be completed by the next chunk.
                        if (this->peek() == nullptr) {
                            return true;
                        }

                        set_numeric_error();
                        return false;
                    }

                    // If the exponent digits ended at EOB, the literal is
                    // already complete. Leave the lexer Normal so a future
                    // adjacent numeric token can be merged normally.
                    this->_state = ZLexerInternalState::Normal;
                    return true;
                }

                case ZLexerInternalState::HexNumber: {
                    const bool consumed =
                        consume_digits(Zaban::Lex::CharUtil::is_hex_digit);

                    if (!consumed) {
                        if (this->peek() == nullptr) {
                            // `0x` can still be completed by another chunk.
                            return true;
                        }

                        set_numeric_error();
                        return false;
                    }

                    if (this->peek() == nullptr) {
                        this->_state = ZLexerInternalState::Normal;
                    }

                    return true;
                }

                case ZLexerInternalState::OctNumber: {
                    const bool consumed =
                        consume_digits(Zaban::Lex::CharUtil::is_oct_digit);

                    if (!consumed) {
                        if (this->peek() == nullptr) {
                            return true;
                        }

                        set_numeric_error();
                        return false;
                    }

                    if (this->peek() == nullptr) {
                        this->_state = ZLexerInternalState::Normal;
                    }

                    return true;
                }

                case ZLexerInternalState::BinNumber: {
                    const bool consumed =
                        consume_digits(Zaban::Lex::CharUtil::is_bin_digit);

                    if (!consumed) {
                        if (this->peek() == nullptr) {
                            return true;
                        }

                        set_numeric_error();
                        return false;
                    }

                    if (this->peek() == nullptr) {
                        this->_state = ZLexerInternalState::Normal;
                    }

                    return true;
                }

                default:
                    return true;
            }
        }
    }

    void ZLexer::skip_trivial() {
        ZLexerBufferType::const_pointer p  = nullptr;
        ZLexerBufferType::value_type    p0 = 0;
        ZLexerBufferType::value_type    p1 = 0;

        for (; this->_buffer_it != this->_buffer.end();) {
            // Reset to normal state.
            this->_state = ZLexerInternalState::Normal;

            p0 = (p = this->peek()) == nullptr ? 0 : *p;

            /* this is unlieky because we already check if iterator has not
            encountered end of line. */
            if (0 == p0) Z_UNLIKELY {
                    return;
                }

            if (Zaban::Lex::CharUtil::is_whitespace(p0)) {
                this->advance();
                continue;
            }

            if (this->scan_newline()) {
                continue;
            }

            bool scan_comment_result = this->scan_comment();
            if (!scan_comment_result &&
                this->_state != ZLexerInternalState::Normal)
                Z_UNLIKELY {
                    return;
                }
            else {
                break;
            }
        }
    }

    static std::optional<ZLexerTokenKind> merge(ZLexerTokenKind lhs,
                                                ZLexerTokenKind rhs) {
        auto it = merges.find({lhs, rhs});

        if (it == merges.end()) {
            return std::nullopt;
        }

        return it->second;
    }

    void ZLexer::merge_double_tokens() {
        std::vector<ZLexerTokenType> merged_tokens;
        merged_tokens.reserve(this->_tokens.size());

        for (std::size_t i = 0; i < this->_tokens.size();) {
            if (_tokens[i].kind == ZLexerTokenKind::Eob ||
                _tokens[i].kind == ZLexerTokenKind::Eof) {
                ++i;
                continue;
            }

            auto token = std::move(_tokens[i]);
            ++i;

            while (i < _tokens.size()) {
                if (_tokens[i].kind == ZLexerTokenKind::Eob ||
                    _tokens[i].kind == ZLexerTokenKind::Eof) {
                    break;
                }

                if (token.kind != ZLexerTokenKind::String &&
                    token.range.end + 1 != _tokens[i].range.begin) {
                    break;
                }

                auto merge_kind = merge(token.kind, _tokens[i].kind);

                if (!merge_kind) {
                    break;
                }

                token.kind      = *merge_kind;
                token.range.end = _tokens[i].range.end;

                ++i;
            }

            merged_tokens.push_back(std::move(token));
        }

        if (merged_tokens.size() == 0) {
            merged_tokens.emplace_back(
                ZLexerTokenKind::Eof,
                SourcePositionRange<ZLexerPositionType>(_offset, _offset));
            _tokens = std::move(merged_tokens);
            return;
        }

        auto eof_pos = merged_tokens.back().range.end + 1;
        merged_tokens.emplace_back(
            ZLexerTokenKind::Eof,
            SourcePositionRange<ZLexerPositionType>(eof_pos, eof_pos));

        _tokens = std::move(merged_tokens);
    }

    void ZLexer::concat(const ZLexer& rhs) {
        this->validate(ZLexerInvalidationFlag::NoScan);

        ZLexer copy = rhs;
        if (this->_state != ZLexerInternalState::Normal ||
            copy._start_offset != _offset) {
            copy.invalidate(ZLexerInvalidationFlag::NoScan);
        }

        if (copy.has_flag(ZLexerInvalidationFlag::NoScan)) {
            copy._state        = this->_state;
            copy._offset       = this->_offset;
            copy._start_offset = this->_offset;
            copy._diagnostics._errors |= this->_diagnostics._errors;

            copy.validate(ZLexerInvalidationFlag::NoScan);
        }
        this->_state                   = copy._state;
        this->_diagnostics._scan_count = (this->_diagnostics.get_scan_count() +
                                          copy._diagnostics.get_scan_count());
        this->_diagnostics._concat_count +=
            copy.diagnostics().get_concat_count();
        this->_diagnostics._errors |= copy._diagnostics._errors;
        this->_diagnostics.increment_concat_count();

        _tokens.reserve(_tokens.size() + copy._tokens.size());
        _tokens.insert(_tokens.end(), copy._tokens.begin(), copy._tokens.end());
        this->invalidate(ZLexerInvalidationFlag::NoMergeTokens);
    }

    void ZLexer::concat(ZLexer&& rhs) {
        if (this == &rhs) {
            return;
        }

        this->validate(ZLexerInvalidationFlag::NoScan);

        if (this->_state != ZLexerInternalState::Normal ||
            rhs._start_offset != _offset) {
            rhs.invalidate(ZLexerInvalidationFlag::NoScan);
        }

        if (rhs.has_flag(ZLexerInvalidationFlag::NoScan)) {
            rhs._state        = this->_state;
            rhs._offset       = this->_offset;
            rhs._start_offset = this->_offset;
            rhs._diagnostics._errors |= this->_diagnostics._errors;

            rhs.validate(ZLexerInvalidationFlag::NoScan);
        }
        this->_state                   = rhs._state;
        this->_diagnostics._scan_count = (this->_diagnostics.get_scan_count() +
                                          rhs._diagnostics.get_scan_count());
        this->_diagnostics._concat_count +=
            rhs.diagnostics().get_concat_count();
        this->_diagnostics._errors |= rhs._diagnostics._errors;
        this->_diagnostics.increment_concat_count();

        _tokens.reserve(_tokens.size() + rhs._tokens.size());
        _tokens.insert(_tokens.end(),
                       std::make_move_iterator(rhs._tokens.begin()),
                       std::make_move_iterator(rhs._tokens.end()));
        this->invalidate(ZLexerInvalidationFlag::NoMergeTokens);
    }

    bool ZLexer::scan() {
        if (this->_buffer_it == this->_buffer.end()) {
            return false;
        }

        // Resume a token that was split across input chunks.
        //
        // Numeric continuation is special: the previous lexer already owns
        // the first part of the token, so this lexer emits the continuation
        // as a Numeric token. `merge_double_tokens()` then folds the two
        // adjacent Numeric tokens back into one source range.
        if (this->_state != ZLexerInternalState::Normal) {
            switch (this->_state) {
                case ZLexerInternalState::LineComment:
                    if (!this->scan_double_slash_close_comment()) {
                        return false;
                    }
                    break;

                case ZLexerInternalState::BlockComment:
                    if (!this->scan_until_block_slash_close_comment()) {
                        return false;
                    }
                    break;

                case ZLexerInternalState::SQString:
                case ZLexerInternalState::DQString:
                    if (!this->scan_until_eos()) {
                        return false;
                    }
                    break;

                default:
                    if (this->_state > ZLexerInternalState::STATE_NumStart &&
                        this->_state < ZLexerInternalState::STATE_NumEnd) {
                        const auto continuation_start = this->_offset;

                        if (!this->scan_until_get_numeric()) {
                            return false;
                        }

                        // Only emit a continuation token if this chunk
                        // actually consumed source characters.
                        if (this->_offset > continuation_start) {
                            this->_tokens.emplace_back(
                                ZLexerTokenKind::Numeric,
                                SourcePositionRange<ZLexerPositionType>(
                                    continuation_start, this->_offset - 1));
                        }
                    }
                    break;
            }
        }

        this->invalidate(ZLexerInvalidationFlag::NoMergeTokens);
        this->_diagnostics.increment_scan_count();

        auto add_token = [this](ZLexerTokenKind kind, ZLexerPositionType start,
                                ZLexerPositionType end) {
            this->_tokens.emplace_back(
                kind, SourcePositionRange<ZLexerPositionType>(start, end));
        };

        auto scan_string = [this, &add_token](
                               ZLexerBufferType::value_type quote,
                               ZLexerPositionType           start) {
            add_token(ZLexerTokenKind::String, start, start);
            this->_state = quote == '\'' ? ZLexerInternalState::SQString
                                         : ZLexerInternalState::DQString;
            this->advance();
            return this->scan_until_eos();
        };

        auto scan_number = [this, &add_token](ZLexerPositionType start) {
            const auto first = *this->peek();
            this->advance();
            this->_state = first == '0' ? ZLexerInternalState::ZeroStart
                                        : ZLexerInternalState::Number;

            if (!this->scan_until_get_numeric()) {
                return false;
            }

            add_token(ZLexerTokenKind::Numeric, start, this->_offset - 1);
            return true;
        };

        while (this->_buffer_it != this->_buffer.end()) {
            this->skip_trivial();

            const auto* p = this->peek();
            if (p == nullptr) {
                add_token(ZLexerTokenKind::Eob, this->_offset, this->_offset);
                return false;
            }

            const auto start = this->_offset;
            const auto p0    = *p;
            const auto p1    = this->peek(1) != nullptr ? *this->peek(1) : 0;

            if (p0 == '\'' || p0 == '"') {
                if (!scan_string(p0, start)) {
                    return false;
                }
                continue;
            }

            if (Zaban::Lex::CharUtil::is_digit(p0)) {
                if (!scan_number(start)) {
                    return false;
                }
                continue;
            }

            // A leading-dot floating point literal, e.g. .10.
            if (p0 == '.' && p1 != 0 && Zaban::Lex::CharUtil::is_digit(p1)) {
                this->advance();
                this->_state = ZLexerInternalState::FloatNumber;

                if (!this->scan_until_get_numeric()) {
                    return false;
                }

                add_token(ZLexerTokenKind::Numeric, start, this->_offset - 1);
                continue;
            }

            const auto kind = [p0]() -> std::optional<ZLexerTokenKind> {
                switch (p0) {
                    case '(':
                        return ZLexerTokenKind::LParen;
                    case ')':
                        return ZLexerTokenKind::RParen;
                    case '[':
                        return ZLexerTokenKind::LBrak;
                    case ']':
                        return ZLexerTokenKind::RBrak;
                    case '{':
                        return ZLexerTokenKind::LBrace;
                    case '}':
                        return ZLexerTokenKind::RBrace;
                    case ',':
                        return ZLexerTokenKind::Comma;
                    case ':':
                        return ZLexerTokenKind::Colon;
                    case ';':
                        return ZLexerTokenKind::Semicolon;
                    case '^':
                        return ZLexerTokenKind::Caret;
                    case '~':
                        return ZLexerTokenKind::Tilde;
                    case '.':
                        return ZLexerTokenKind::Dot;
                    case '+':
                        return ZLexerTokenKind::Plus;
                    case '-':
                        return ZLexerTokenKind::Minus;
                    case '*':
                        return ZLexerTokenKind::Asterisk;
                    case '/':
                        return ZLexerTokenKind::Slash;
                    case '%':
                        return ZLexerTokenKind::Percent;
                    case '&':
                        return ZLexerTokenKind::Amp;
                    case '|':
                        return ZLexerTokenKind::Pipe;
                    case '=':
                        return ZLexerTokenKind::Equal;
                    case '!':
                        return ZLexerTokenKind::Exclam;
                    case '?':
                        return ZLexerTokenKind::Qmark;
                    case '<':
                        return ZLexerTokenKind::Lesser;
                    case '>':
                        return ZLexerTokenKind::Greater;
                    case '@':
                        return ZLexerTokenKind::AtSign;
                    default:
                        return std::nullopt;
                }
            }();

            if (kind.has_value()) {
                add_token(*kind, start, start);
            }

            this->advance();
        }

        return true;
    }

    std::vector<ZLexerTokenType> ZLexer::finalize() {
        this->validate_all();
        if (ZLexerInternalState::Normal != this->_state) {
            switch (this->_state) {
                case ZLexerInternalState::Number:
                case ZLexerInternalState::FloatNumber:
                    // These states are valid at a hard EOF. They are kept
                    // alive during incremental scanning only so the next
                    // chunk can extend the literal.
                    this->_state = ZLexerInternalState::Normal;
                    return this->_tokens;

                case ZLexerInternalState::LineComment:
                case ZLexerInternalState::BlockComment:
                    this->_diagnostics._errors =
                        set(this->_diagnostics._errors,
                            ZLexerErrorFlag::UnterminatedComment);
                    break;

                case ZLexerInternalState::SQString:
                case ZLexerInternalState::DQString:
                    this->_diagnostics._errors =
                        set(this->_diagnostics._errors,
                            ZLexerErrorFlag::UnterminatedString);
                    break;

                case ZLexerInternalState::Error:
                    break;

                default:
                    // Numeric prefix/exponent states which still require
                    // characters are incomplete at a hard EOF.
                    break;
            }

            return {};
        }

        return this->_tokens;
    }

    LexerDiagnostics& ZLexer::diagnostics() {
        return this->_diagnostics;
    }

    void ZLexer::invalidate(const ZLexerInvalidationFlag flag) {
        if (flag == ZLexerInvalidationFlag::NoScan) {
            this->_buffer_it = this->_buffer.begin();
            this->_tokens.clear();
            this->_offset = this->_start_offset;
        }
        this->_flags = set(this->_flags, flag);
    }

    bool ZLexer::has_flag(const ZLexerInvalidationFlag flag) {
        return has(this->_flags, flag);
    }

    void ZLexer::validate(const ZLexerInvalidationFlag flag) {
        switch (flag) {
            case ZLexerInvalidationFlag::None: {
                return;
            }
            case ZLexerInvalidationFlag::NoScan: {
                if (has_flag(ZLexerInvalidationFlag::NoScan)) {
                    if (this->scan()) {
                        this->_flags =
                            unset(this->_flags, ZLexerInvalidationFlag::NoScan);
                        this->_diagnostics._errors = ZLexerErrorFlag::None;
                    }
                }
            } break;
            case ZLexerInvalidationFlag::NoMergeTokens: {
                if (has_flag(ZLexerInvalidationFlag::NoMergeTokens)) {
                    this->merge_double_tokens();
                    this->_flags = unset(this->_flags,
                                         ZLexerInvalidationFlag::NoMergeTokens);
                }
            } break;
        }
    }

    void ZLexer::validate_all() {
        this->validate(ZLexerInvalidationFlag::NoScan);
        this->validate(ZLexerInvalidationFlag::NoMergeTokens);
    }

}  // namespace Z::Zaban::Langs::ZLang
