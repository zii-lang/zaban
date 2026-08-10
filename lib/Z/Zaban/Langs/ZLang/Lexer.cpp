#include <Z/Zaban/Config.hpp>
#include <Z/Zaban/Langs/ZLang/Lexer.hpp>
#include <Z/Zaban/Lex/CharUtil.hpp>
#include <Z/Zaban/Lex/ScanUtil.hpp>
#include <array>
#include <memory>
#include <optional>
#include <tuple>
#include <unordered_map>

namespace Z::Zaban::Langs::ZLang {
    const static std::unordered_map<std::string, ZLexerTokenKind>
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
            auto lhs = static_cast<std::size_t>(pair.lhs);
            auto rhs = static_cast<std::size_t>(pair.rhs);

            return (lhs << 32) ^ rhs;
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

#define ZADD_TOKEN(kind)        \
    this->_tokens.emplace_back( \
        kind, SourcePositionRange<ZLexerPositionType>(start, end))

    bool ZLexer::scan() {
        if (this->_buffer_it == this->_buffer.end()) {
            return false;
        }

        // catchup.
        if (ZLexerInternalState::Normal != this->_state) {
            if (ZLexerInternalState::LineComment == this->_state) {
                if (!this->scan_double_slash_close_comment()) {
                    return false;
                }
            } else if (ZLexerInternalState::BlockComment == this->_state) {
                if (!this->scan_until_block_slash_close_comment()) {
                    return false;
                }
            } else if (ZLexerInternalState::SQString == this->_state ||
                       ZLexerInternalState::DQString == this->_state) {
                if (!this->scan_until_eos()) {
                    return false;
                }
            }
        }

        ZLexerBufferType::value_type p0 = 0;
        ZLexerBufferType::value_type p1 = 0;

        this->invalidate(ZLexerInvalidationFlag::NoMergeTokens);
        this->_diagnostics.increment_scan_count();

        for (; this->_buffer_it != this->_buffer.end();) {
            this->skip_trivial();
            ZLexerPositionType start = this->_offset;
            ZLexerPositionType end   = this->_offset;

            ZLexerBufferType::const_pointer p = this->peek();
            if (nullptr == p) Z_UNLIKELY {
                    ZADD_TOKEN(ZLexerTokenKind::Eob);
                    return false;
                }

            p0 = *p;
            p1 = (p = this->peek(1)) == nullptr ? 0 : *p;

            switch (p0) {
                case '(':
                    ZADD_TOKEN(ZLexerTokenKind::LParen);
                    break;
                case ')':
                    ZADD_TOKEN(ZLexerTokenKind::RParen);
                    break;
                case '[':
                    ZADD_TOKEN(ZLexerTokenKind::LBrak);
                    break;
                case ']':
                    ZADD_TOKEN(ZLexerTokenKind::RBrak);
                    break;
                case '{':
                    ZADD_TOKEN(ZLexerTokenKind::LBrace);
                    break;
                case '}':
                    ZADD_TOKEN(ZLexerTokenKind::RBrace);
                    break;
                case ',':
                    ZADD_TOKEN(ZLexerTokenKind::Comma);
                    break;
                case ':':
                    ZADD_TOKEN(ZLexerTokenKind::Colon);
                    break;
                case ';':
                    ZADD_TOKEN(ZLexerTokenKind::Semicolon);
                    break;
                case '^':
                    ZADD_TOKEN(ZLexerTokenKind::Caret);
                    break;
                case '~':
                    ZADD_TOKEN(ZLexerTokenKind::Tilde);
                    break;
                case '.':
                    ZADD_TOKEN(ZLexerTokenKind::Dot);
                    break;
                case '+':
                    ZADD_TOKEN(ZLexerTokenKind::Plus);
                    break;
                case '-':
                    ZADD_TOKEN(ZLexerTokenKind::Minus);
                    break;
                case '*':
                    ZADD_TOKEN(ZLexerTokenKind::Asterisk);
                    break;
                case '/':
                    ZADD_TOKEN(ZLexerTokenKind::Slash);
                    break;
                case '%':
                    ZADD_TOKEN(ZLexerTokenKind::Percent);
                    break;
                case '&':
                    ZADD_TOKEN(ZLexerTokenKind::Amp);
                    break;
                case '|':
                    ZADD_TOKEN(ZLexerTokenKind::Pipe);
                    break;
                case '=':
                    ZADD_TOKEN(ZLexerTokenKind::Equal);
                    break;
                case '!':
                    ZADD_TOKEN(ZLexerTokenKind::Exclam);
                    break;
                case '?':
                    ZADD_TOKEN(ZLexerTokenKind::Qmark);
                    break;
                case '<':
                    ZADD_TOKEN(ZLexerTokenKind::Lesser);
                    break;
                case '>':
                    ZADD_TOKEN(ZLexerTokenKind::Greater);
                    break;
                case '@':
                    ZADD_TOKEN(ZLexerTokenKind::AtSign);
                    break;
                default:
                    break;
            }

            if ('\'' == p0 || '"' == p0) {
                ZADD_TOKEN(ZLexerTokenKind::String);
                switch (p0) {
                    case '\'':
                        this->_state = ZLexerInternalState::SQString;
                        break;
                    case '"':
                        this->_state = ZLexerInternalState::DQString;
                        break;
                    default:
                        break;
                }
                this->advance();
                if (!this->scan_until_eos()) {
                    return false;
                }
                continue;
            }

            this->advance();
        }
        return true;
    }
#undef ZADD_TOKEN

    std::vector<ZLexerTokenType> ZLexer::finalize() {
        this->validate_all();
        if (ZLexerInternalState::Normal != this->_state) {
            switch (this->_state) {
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
                default:
                    this->_diagnostics._errors = ZLexerErrorFlag::None;
                    break;
            }
            // TODO: set diagnostics for error
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
