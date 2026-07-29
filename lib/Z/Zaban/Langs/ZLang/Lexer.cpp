#include <Z/Zaban/Langs/ZLang/Lexer.hpp>
#include <memory>
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

    ZLexer::ZLexer(ZLexerBufferType& buffer) :
        Zaban::Lex::Lexer<ZLexerTokenType, ZLexerPositionType,
                          ZLexerBufferType>(buffer),
        _buffer_it(buffer.begin()) {};

    void ZLexer::set_buffer(ZLexerBufferType& buffer) {
        this->_buffer    = buffer;
        this->_buffer_it = this->_buffer.begin();
    }

    bool ZLexer::analyze() {
        return false;
    }

    std::vector<ZLexerTokenType> ZLexer::finalize() {
        return std::vector<ZLexerTokenType>();
    }

    LexerDiagnostics ZLexer::diagnostics() {
        return LexerDiagnostics();
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

    //     ZLexerBufferType::const_pointer ZLexer::get() {
    //         ZLexerBufferType::const_pointer ch = this->peek();
    //         if (ch != nullptr) {
    //             this->advance();
    //         }
    //         return ch;
    //     }

    //     void ZLexer::skip_trivial() {
    //         ZLexerBufferType::const_pointer p  = nullptr;
    //         char                            p0 = 0;
    //         char                            p1 = 0;

    //         for (; this->_buffer_it != this->current_buffer.end();
    //              ++this->_buffer_it) {
    //             p0 = (p = this->peek()) == nullptr ? 0 : *p;
    //             if (p0 == 0) return;

    //             if (Zaban::CharUtil::is_whitespace(p0)) {
    //                 ZLexerPositionType count =
    //                 this->conditional_line_update(p0); this->advance(count);
    //                 continue;
    //             }

    //             p1 = (p = this->peek(1)) == nullptr ? 0 : *p;
    //             if (p1 == 0) return;

    //             if (!Zaban::ScanUtil::is_either_slash_comment(p0, p1)) {
    //                 break;
    //             }

    //             this->advance(2);

    //             while (this->_buffer_it != this->current_buffer.end()) {
    //                 ZLexerPositionType newline_count = 0;
    //                 if (Zaban::ScanUtil::is_double_slash_comment(p0, p1)) {
    //                     ZLexerBufferType::const_reference ch = this->get();
    //                     while (this->_buffer_it !=
    //                     this->current_buffer.end()) {
    //                         this->_buffer_it++;
    //                         this->current_offset++;
    //                         if (0 != (newline_count =
    //                                       this->conditional_line_update(ch)))
    //                                       {
    //                             this->_buffer_it += newline_count;
    //                             this->current_offset += newline_count;
    //                             break;
    //                         }
    //                     }
    //                 } else {
    //                     p0 = this->peek(buffer_it);
    //                     p1 = this->peek(buffer_it, 1);

    //                     if (p0 == '*' && p1 == '/') {
    //                         this->advance_offset(2);
    //                         buffer_it += 2;
    //                         break;
    //                     } else {
    //                         if (0 != (newline_count =
    //                                       this->conditional_line_update(p0)))
    //                                       {
    //                             buffer_it += newline_count;
    //                             this->advance_offset(newline_count);
    //                         }
    //                         buffer_it++;
    //                         this->advance_offset(1);
    //                     }
    //                 }
    //             }
    //         }
    //         return buffer_it;
    //     }

    // #define TOKEN(kind, start_offset, start_file, end_offset, end_file) \
    //     ZLexerTokenType(kind, \
    //                     SourceLocation<ZLexerPositionType,
    //                     ZLexerFileRefType>( \
    //                         start_offset, start_file), \
    //                     SourceLocation<ZLexerPositionType,
    //                     ZLexerFileRefType>( \
    //                         end_offset, end_file))

    //     ZLexerTokenType ZLexer::get_token() {
    //         for (; this->_buffer_it != this->current_buffer.end();
    //              ++this->_buffer_it) {
    //             this->skip_trivial();
    //             const ZLexerPositionType        start_offset =
    //             this->current_offset; ZLexerBufferType::const_pointer ch =
    //             this->get();

    //             switch (*ch) {
    //                 case '(':
    //                     return TOKEN(TokenKind::LParen, start_offset,
    //                                  this->current_file,
    //                                  this->current_offset,
    //                                  this->current_file);
    //                 default:
    //                     std::cout << "char " << ch << std::endl;
    //             }
    //         }

    //         return ZLexerTokenType(
    //             TokenKind::Dummy,
    //             SourceLocation<ZLexerPositionType, ZLexerFileRefType>(
    //                 0, this->current_file),
    //             SourceLocation<ZLexerPositionType, ZLexerFileRefType>(
    //                 0, this->current_file));
    //     }

    // #undef TOKEN

    //     bool ZLexer::advance() {
    //         return this->advance(1);
    //     }

    //     bool ZLexer::advance(ZLexerPositionType offset) {
    //         if (this->_buffer_it + offset >= this->current_buffer.end()) {
    //             return false;
    //         }
    //         this->current_offset += offset;
    //         this->_buffer_it += offset;
    //         return true;
    //     }

    //     ZLexerPositionType ZLexer::conditional_line_update(const char ch) {
    //         ZLexerPositionType              newline_count = 0;
    //         ZLexerBufferType::const_pointer next_p        = this->peek(0);
    //         if (next_p == nullptr) {
    //             return newline_count;
    //         }
    //         if (Z::Zaban::ScanUtil::is_newline_seq(ch, *next_p,
    //         &newline_count)) {
    //             this->set_current_line(this->get_current_line() +
    //             newline_count);
    //         }
    //         return newline_count;
    //     }
}  // namespace Z::Zaban::Langs::ZLang
