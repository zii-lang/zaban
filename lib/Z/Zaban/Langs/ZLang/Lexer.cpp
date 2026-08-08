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
    };

    ZLexer::ZLexer(ZLexerBufferType& buffer) :
        Zaban::Lex::Lexer<ZLexerTokenType, ZLexerPositionType,
                          ZLexerBufferType>(buffer),
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
                this->set_lexer_state(ZLexerInternalState::Whitespace);
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
                    this->_error = ZLexerError::UnterminatedComment;
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

        for (auto i = 0; i < this->_tokens.size(); ++i) {
            if (_tokens[i].kind == ZLexerTokenKind::Eob ||
                _tokens[i].kind == ZLexerTokenKind::Eof) {
                continue;
            }

            if (i + 1 == this->_tokens.size()) {
                merged_tokens.push_back(this->_tokens[i]);
                break;
            }

            if (auto merge_kind =
                    merge(this->_tokens[i].kind, this->_tokens[i + 1].kind)) {
                ZLexerTokenType token = std::move(this->_tokens[i]);
                if (this->_tokens[i].range.end + 1 ==
                    this->_tokens[i + 1].range.begin) {
                    token.kind        = *merge_kind;
                    token.range.begin = this->_tokens[i].range.begin;
                    token.range.end   = this->_tokens[i + 1].range.end;
                    ++i;
                }
                merged_tokens.push_back(token);
                continue;
            }

            merged_tokens.push_back(std::move(_tokens[i]));
        }
        merged_tokens.emplace_back(ZLexerTokenKind::Eof,
                                   SourcePositionRange<ZLexerPositionType>(
                                       this->_offset, this->_offset));
        this->_tokens = std::move(merged_tokens);
    }

    void ZLexer::concat(const ZLexer& rhs) {
        if (this->has_flag(ZLexerInvalidationFlag::NoScan)) {
            this->scan();
        }

        ZLexer copy = rhs;

        copy._state        = _state;
        copy._error        = _error;
        copy._offset       = _offset;
        copy._start_offset = _offset;

        copy.invalidate(ZLexerInvalidationFlag::NoScan);
        copy.scan();

        _tokens.reserve(_tokens.size() + copy._tokens.size());
        _tokens.insert(_tokens.end(), copy._tokens.begin(), copy._tokens.end());
    }

    void ZLexer::concat(ZLexer&& rhs) {
        if (this == &rhs) {
            return;
        }
        if (!this->has_flag(ZLexerInvalidationFlag::NoScan)) {
            this->scan();
        }

        rhs._state        = _state;
        rhs._error        = _error;
        rhs._offset       = _offset;
        rhs._start_offset = _offset;

        rhs.invalidate(ZLexerInvalidationFlag::NoScan);
        rhs.scan();

        _tokens.reserve(_tokens.size() + rhs._tokens.size());
        _tokens.insert(_tokens.end(),
                       std::make_move_iterator(rhs._tokens.begin()),
                       std::make_move_iterator(rhs._tokens.end()));
    }

#define ZADD_TOKEN(kind)        \
    this->_tokens.emplace_back( \
        kind, SourcePositionRange<ZLexerPositionType>(start, end))

    bool ZLexer::scan() {
        if (this->_buffer_it == this->_buffer.end()) {
            return false;
        }

        if (ZLexerInternalState::Normal != this->_state) {
            if (ZLexerInternalState::LineComment == this->_state) {
                if (!this->scan_double_slash_close_comment()) {
                    return false;
                }
            } else if (ZLexerInternalState::BlockComment == this->_state) {
                if (!this->scan_until_block_slash_close_comment()) {
                    return false;
                }
            } else if (ZLexerInternalState::Whitespace == this->_state) {
                this->_state = ZLexerInternalState::Normal;
            }
        }

        ZLexerBufferType::value_type p0 = 0;
        ZLexerBufferType::value_type p1 = 0;

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
                case '=':
                    ZADD_TOKEN(ZLexerTokenKind::Equal);
                    break;
                case '<':
                    ZADD_TOKEN(ZLexerTokenKind::Lesser);
                    break;
                case '>':
                    ZADD_TOKEN(ZLexerTokenKind::Greater);
                    break;
                default:
                    break;
            }
            this->advance();
        }
        this->invalidate(ZLexerInvalidationFlag::NoMergeTokens);
        return true;
    }
#undef ZADD_TOKEN

    std::vector<ZLexerTokenType> ZLexer::finalize() {
        validate();
        return this->_tokens;
    }

    LexerDiagnostics ZLexer::diagnostics() {
        return LexerDiagnostics();
    }

    void ZLexer::invalidate(const ZLexerInvalidationFlag flag) {
        if (flag == ZLexerInvalidationFlag::NoScan) {
            this->_buffer_it = this->_buffer.begin();
            this->_tokens.clear();
            this->_offset = this->_start_offset;
        }
        this->_flags |= flag;
    }

    bool ZLexer::has_flag(const ZLexerInvalidationFlag flag) {
        return (this->_flags & flag) == flag;
    }

    void ZLexer::validate() {
        if (has_flag(ZLexerInvalidationFlag::NoScan)) {
            this->scan();
            this->_flags &= ZLexerInvalidationFlag::NoScan;
        }

        if (has_flag(ZLexerInvalidationFlag::NoMergeTokens)) {
            this->merge_double_tokens();
            this->_flags &= ZLexerInvalidationFlag::NoMergeTokens;
        }

        // assert(this->_flags == ZLexerInvalidationFlag::None);
    }

}  // namespace Z::Zaban::Langs::ZLang
