#include <Z/Zaban/Langs/CLang/Lexer.hpp>
#include <memory>
#include <string>
#include <unordered_map>

#include "Z/Zaban/Langs/CLang/TokenKind.hpp"
#include "Z/Zaban/Lex/LexerDiagnostics.hpp"

namespace Z::Zaban::Langs::CLang {
    // TODO: string_view and hashing maybe? should improve performance
    const static std::unordered_map<std::string, CLexerTokenKind>
        CLangKeywords = {
            {"alignas", TokenKind::Alignas},
            {"_Alignas", TokenKind::Alignas},
            {"alignof", TokenKind::Alignof},
            {"_Alignof", TokenKind::Alignof},
            {"_Atomic", TokenKind::Atomic},
            {"auto", TokenKind::Auto},
            {"_BitInt", TokenKind::BitInt},
            {"bool", TokenKind::Bool},
            {"_Bool", TokenKind::Bool},
            {"break", TokenKind::Break},
            {"case", TokenKind::Case},
            {"char", TokenKind::Char},
            {"_Complex", TokenKind::Complex},
            {"const", TokenKind::Const},
            {"constexpr", TokenKind::Constexpr},
            {"continue", TokenKind::Continue},
            {"_Decimal32", TokenKind::Decimal32},
            {"_Decimal64", TokenKind::Decimal64},
            {"_Decimal128", TokenKind::Decimal128},
            {"default", TokenKind::Default},
            {"do", TokenKind::Do},
            {"double", TokenKind::Double},
            {"else", TokenKind::Else},
            {"enum", TokenKind::Enum},
            {"extern", TokenKind::Extern},
            {"false", TokenKind::False},
            {"float", TokenKind::Float},
            {"for", TokenKind::For},
            {"_Generic", TokenKind::Generic},
            {"goto", TokenKind::Goto},
            {"if", TokenKind::If},
            {"_Imaginary", TokenKind::Imaginary},
            {"inline", TokenKind::Inline},
            {"int", TokenKind::Int},
            {"long", TokenKind::Long},
            {"_Noreturn", TokenKind::Noreturn},
            {"nullptr", TokenKind::Nullptr},
            {"register", TokenKind::Register},
            {"restrict", TokenKind::Restrict},
            {"return", TokenKind::Return},
            {"short", TokenKind::Short},
            {"signed", TokenKind::Signed},
            {"sizeof", TokenKind::Sizeof},
            {"static", TokenKind::Static},
            {"static_assert", TokenKind::StaticAssert},
            {"_Static_assert", TokenKind::StaticAssert},
            {"struct", TokenKind::Struct},
            {"switch", TokenKind::Switch},
            {"thread_local", TokenKind::ThreadLocal},
            {"_Thread_local", TokenKind::ThreadLocal},
            {"true", TokenKind::True},
            {"typedef", TokenKind::Typedef},
            {"typeof", TokenKind::Typeof},
            {"typeof_unqual", TokenKind::TypeofUnqual},
            {"union", TokenKind::Union},
            {"unsigned", TokenKind::Unsigned},
            {"void", TokenKind::Void},
            {"volatile", TokenKind::Volatile},
            {"while", TokenKind::While},
    };

    CLexer::CLexer(CLexerBufferType& buffer) :
        Z::Zaban::Lex::Lexer<CLexerTokenType, CLexerPositionType,
                             CLexerBufferType>(buffer),
        _buffer_it(this->_buffer.begin()) {
        this->_prev_buffer_last = this->_buffer.data() + this->_buffer.size();
    }

    void CLexer::set_buffer(CLexerBufferType& buffer) {
        // accouting for bytes of the outgoing chunk that were never consumed
        // expecting this to be 0 usually and non zero on early stops!
        this->_offset += static_cast<CLexerPositionType>(this->_buffer.end() -
                                                         this->_buffer_it);
        this->_contiguous = this->_prev_buffer_last != nullptr &&
                            this->_prev_buffer_last == buffer.data();
        // saving this chunk ending for later calls
        this->_prev_buffer_last = buffer.data() + buffer.size();

        this->_buffer    = buffer;
        this->_buffer_it = this->_buffer.begin();
    }

    // TODO:
    bool CLexer::analyze() {
        return false;
    }

    // TODO:
    std::vector<CLexerTokenType> CLexer::finalize() {
        return std::vector<CLexerTokenType>();
    }

    LexerDiagnostics CLexer::diagnostics() {
        return LexerDiagnostics();
    }

    // TODO:
    void CLexer::push_token(CLexerTokenKind token) {
    }
    // TODO:
    void CLexer::set_error(CLexerError err) {
    }

    // TODO:
    bool CLexer::advance() {
        return this->advance(1);
    }
    bool CLexer::advance(CLexerPositionType offset) {
        if (offset > static_cast<CLexerPositionType>(this->_buffer.end() -
                                                     this->_buffer_it))
            return false;

        this->_offset += offset;
        this->_buffer_it += offset;
        return true;
    }

    CLexerPositionType CLexer::get_offset() {
        return this->_offset;
    }

    bool CLexer::eof() const {
        return (this->_buffer_it >= this->_buffer.end());
    }

    CLexerBufferType::const_pointer CLexer::peek() const {
        return this->peek(0);
    }
    CLexerBufferType::const_pointer CLexer::peek(
        const CLexerPositionType offset) const {
        if (this->_buffer_it + offset >= this->_buffer.end()) {
            return nullptr;
        }
        return std::to_address(this->_buffer_it + offset);
    }

    CLexerBufferType::const_pointer CLexer::get() {
        CLexerBufferType::const_pointer ch = this->peek();
        if (ch != nullptr) {
            advance();
        }
        return ch;
    }

}  // namespace Z::Zaban::Langs::CLang
