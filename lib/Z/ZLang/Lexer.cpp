#include <Z/Zaban/ZLang/Lexer.hpp>
#include <unordered_map>

namespace Z::Zaban::ZLang {
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
        Z::Zaban::Lexer<ZLexerTokenType, ZLexerPositionType, ZLexerFileRefType,
                        ZLexerBufferType>(buffer, file) {
        this->current_buffer           = buffer;
        this->current_file             = file;
        this->current_line             = 1;
        this->current_offset           = 0;
        this->offset_in_current_buffer = 0;
    }

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

    ZLexerTokenType ZLexer::get_token() {
        while (true) {
        }
    }
}  // namespace Z::Zaban::ZLang
