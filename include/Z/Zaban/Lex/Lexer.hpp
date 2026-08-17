#pragma once

#include <Z/Zaban/Lex/LexerError.hpp>
#include <concepts>
#include <memory>
#include <vector>

namespace Z::Zaban::Lex {
    /**
     * @brief Base interface for lexical analyzers.
     *
     * Lexer defines the common interface for converting a source buffer into
     * a sequence of tokens. Implementations are responsible for maintaining
     * the current reading position, reporting diagnostics, and producing
     * finalized token streams.
     *
     * @tparam T Token type produced by the lexer.
     * @tparam P Integral type used for source positions and offsets.
     * @tparam B Source buffer type (for example, `std::string_view`).
     */
    template<typename T, typename P, typename B>
        requires std::integral<P>
    class Lexer {
       private:
        using LexerTokenType    = T;
        using LexerBufferType   = B;
        using LexerPositionType = P;

        /// Whether the lexer operates in strict mode.
        bool _strict = false;

       protected:
        /// Source buffer currently being analyzed.
        LexerBufferType _buffer;

        /// Current offset within the source buffer.
        LexerPositionType _offset;
        /// Offset of start when reset.
        LexerPositionType _start_offset = 0;

       public:
        /**
         * @brief Constructs a lexer for the given source buffer.
         *
         * @param buffer Source buffer to analyze.
         */
        explicit Lexer(LexerBufferType& buffer) :
            _buffer(buffer), _offset(0) {};

        /**
         * @brief Constructs a lexer for the given source buffer.
         *
         * @param buffer Source buffer to analyze.
         * @param start_pos Offset from the main buffer.
         */
        explicit Lexer(LexerBufferType& buffer, LexerPositionType start_pos) :
            _buffer(buffer), _offset(start_pos), _start_offset(start_pos) {};

        /// Virtual destructor.
        virtual ~Lexer() = default;

        /**
         * @brief Replaces the current source buffer.
         *
         * Implementations should reset any state necessary to begin lexing
         * the new buffer.
         *
         * @param buffer New source buffer.
         */
        virtual void set_buffer(LexerBufferType& buffer) {
            this->_buffer = buffer;
        };

        /**
         * @brief Returns the current absolute offset within the source buffer.
         *
         * Implementations should provide the current byte or character offset
         * corresponding to the lexer state.
         *
         * @return Current source offset.
         */
        virtual LexerPositionType get_offset() = 0;

        /**
         * @brief Sets the current source offset.
         *
         * @param offset New offset within the source buffer.
         */
        virtual void set_offset(LexerPositionType offset) {
            this->_offset = offset;
        };

        /**
         * @brief Enables or disables strict lexing mode.
         *
         * In strict mode, the lexer stops processing as soon as it encounters
         * a recoverable lexical error instead of attempting error recovery.
         *
         * @param strict Whether strict mode should be enabled.
         */
        virtual void set_strict(bool strict) {
            this->_strict = strict;
        };

        /**
         * @brief Performs lexical analysis on the current source buffer.
         *
         * The lexer processes the available input and accumulates tokens and
         * diagnostics internally.
         *
         * @return `true` if the current buffer represents a complete source.
         * @return `false` if additional input is required to complete lexing
         *         (for example, an unterminated string or block comment).
         */
        virtual bool scan() = 0;

        /**
         * @brief Finalizes lexical analysis and returns the generated tokens.
         *
         * This function transfers or returns the complete token sequence
         * collected by the lexer.
         *
         * @return Vector containing all generated tokens.
         */
        virtual std::vector<LexerTokenType> finalize() = 0;

        /**
         * @brief Returns the diagnostics generated during lexing.
         *
         * Diagnostics may include lexical errors, warnings, and other
         * recoverable issues encountered while analyzing the source.
         *
         * @return Collection of lexer diagnostics.
         */
        virtual LexerDiagnostics& diagnostics() = 0;
    };
}  // namespace Z::Zaban::Lex
