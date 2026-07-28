#include <Z/Zaban/CharUtil.hpp>
#include <Z/Zaban/ScanUtil.hpp>
#include <Z/Zaban/SourcePosition.hpp>
#include <Z/Zaban/ZLang/Lexer.hpp>
#include <Z/Zaban/ZLang/Token.hpp>
#include <Z/Zaban/ZLang/TokenKind.hpp>
#include <iostream>  // Remove this
#include <unordered_map>

namespace Z::Zaban::ZLang {
    using ZSourceLocation =
        SourceLocation<ZLexerPositionType, ZLexerFileRefType>;
    using ZSourceRange = SourceRange<ZLexerPositionType, ZLexerPositionType>;

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

    ZLexer::ZLexer(ZLexerBufferType& buffer, ZLexerFileRefType file) :
        Lexer(buffer, std::move(file)) {};

    void ZLexer::swap_buffer(ZLexerBufferType& buffer) {
        this->current_buffer           = buffer;
        this->offset_in_current_buffer = 0;
    }

    ZLexerFileRefType ZLexer::get_current_file() {
        return this->current_file;
    }

    ZLexerPositionType ZLexer::get_current_line() {
        return this->current_line;
    }

    ZLexerPositionType ZLexer::get_current_offset() {
        return this->current_offset;
    }

    void ZLexer::set_current_file(ZLexerFileRefType file) {
        this->current_file = file;
    }

    void ZLexer::set_current_line(ZLexerPositionType line) {
        this->current_line = line;
    }

    void ZLexer::set_current_offset(ZLexerPositionType offset) {
        this->current_offset = offset;
    }

#define TOKEN(kind, start_offset, start_file, end_offset, end_file)        \
    ZLexerTokenType(kind,                                                  \
                    SourceLocation<ZLexerPositionType, ZLexerFileRefType>( \
                        start_offset, start_file),                         \
                    SourceLocation<ZLexerPositionType, ZLexerFileRefType>( \
                        end_offset, end_file))

    ZLexerTokenType ZLexer::get_token() {
        for (auto buffer_it =
                 this->current_buffer.begin() + this->offset_in_current_buffer;
             buffer_it != this->current_buffer.end(); ++buffer_it) {
            buffer_it = this->skip_trivial(buffer_it);
            const ZLexerPositionType start_offset = this->current_offset;
            ZLexerBufferType::const_reference ch  = *buffer_it;
            this->advance_offset();

            switch (ch) {
                case '(':
                    return TOKEN(TokenKind::LParen, start_offset,
                                 this->current_file, this->current_offset,
                                 this->current_file);
                default:
                    std::cout << "char " << ch << std::endl;
            }
        }

        return ZLexerTokenType(
            TokenKind::Dummy,
            SourceLocation<ZLexerPositionType, ZLexerFileRefType>(
                0, this->current_file),
            SourceLocation<ZLexerPositionType, ZLexerFileRefType>(
                0, this->current_file));
    }

#undef TOKEN

    ZLexerBufferType::const_reference ZLexer::peek(
        const ZLexerPositionType offset) const {
        return this->current_buffer.at(offset);
    }

    ZLexerBufferType::const_reference ZLexer::peek(
        ZLexerBufferType::const_iterator it) const {
        return *it;
    }

    ZLexerBufferType::const_reference ZLexer::peek(
        ZLexerBufferType::const_iterator it, ZLexerPositionType offset) const {
        if (it + offset >= this->current_buffer.end()) {
            return *(this->current_buffer.end() - 1);
        }
        return *(it + offset);
    }

    ZLexerBufferType::const_iterator ZLexer::skip_trivial(
        ZLexerBufferType::const_iterator it) {
        char p0 = 0;
        char p1 = 0;

        // TODO: left here, have error.
        ZLexerBufferType::const_iterator buffer_it = it;
        for (; buffer_it != this->current_buffer.end(); ++buffer_it) {
            p0 = this->peek(buffer_it);

            if (Zaban::CharUtil::is_whitespace(p0)) {
                this->advance_offset();
                this->conditional_line_update(p0);
                continue;
            }

            p1 = this->peek(buffer_it, 1);

            if (!Zaban::ScanUtil::is_either_slash_comment(p0, p1)) {
                break;
            }

            this->advance_offset(2);
            buffer_it += 2;

            while (buffer_it != this->current_buffer.end()) {
                ZLexerPositionType newline_count = 0;
                if (Zaban::ScanUtil::is_double_slash_comment(p0, p1)) {
                    ZLexerBufferType::const_reference ch = *buffer_it;
                    while (buffer_it != this->current_buffer.end()) {
                        this->advance_offset();
                        buffer_it++;
                        if (0 != (newline_count =
                                      this->conditional_line_update(ch))) {
                            buffer_it += newline_count;
                            break;
                        }
                    }
                } else {
                    p0 = this->peek(buffer_it);
                    p1 = this->peek(buffer_it, 1);

                    if (p0 == '*' && p1 == '/') {
                        this->advance_offset(2);
                        buffer_it += 2;
                        break;
                    } else {
                        if (0 != (newline_count =
                                      this->conditional_line_update(p0))) {
                            buffer_it += newline_count;
                            this->advance_offset(newline_count);
                        }
                        buffer_it++;
                        this->advance_offset(1);
                    }
                }
            }
        }
        return buffer_it;
    }

    ZLexerPositionType ZLexer::conditional_line_update(const char ch) {
        ZLexerPositionType newline_count = 0;
        if (Z::Zaban::ScanUtil::is_newline_seq(ch, this->peek(0),
                                               &newline_count)) {
            this->set_current_line(this->get_current_line() + newline_count);
        }
        return newline_count;
    }

    void ZLexer::advance_offset() {
        this->current_offset++;
        this->offset_in_current_buffer++;
    }

    void ZLexer::advance_offset(const ZLexerPositionType offset) {
        this->current_offset += offset;
        this->offset_in_current_buffer += offset;
    }

}  // namespace Z::Zaban::ZLang
