#pragma once

#include <memory>

namespace Z::Zaban {
    /** @brief Represents a position inside a source file.
     *
     * SourceLocation identifies a specific location in source code using an
     * offset and a source file reference.
     *
     * The file reference type is generic and may represent different forms of
     * source file identification, such as a numeric file ID, a file path
     * string, or a custom source file handle.
     *
     * @tparam T Type used to represent the source offset.
     * @tparam F Type used to reference the source file.
     */
    template<typename T, typename F>
    class SourceLocation {
        using offset_type          = T;
        using source_file_ref_type = F;

       public:
        const offset_type          offset;
        const source_file_ref_type file_ref;
        /**
         * @brief Creates a source location.
         *
         * @param offset Offset within the source file.
         * @param file_ref Reference identifying the source file.
         */
        explicit SourceLocation(offset_type          offset,
                                source_file_ref_type file_ref) :
            offset(offset), file_ref(std::move(file_ref)) {
        }

        SourceLocation(const SourceLocation&) = default;

        SourceLocation& operator=(const SourceLocation&) = delete;

        ~SourceLocation() = default;
    };

    /** @brief Represents a range between two source locations.
     *
     * SourceRange describes a continuous region in a source file using a
     * starting and ending SourceLocation.
     *
     * A source range is commonly used by tokens, AST nodes, and diagnostics to
     * track the portion of source code associated with a compiler entity.
     *
     * @tparam T Type used to represent the source offset.
     * @tparam F Type used to reference the source file.
     */
    template<typename T, typename F>
    struct SourceRange {
       public:
        SourceLocation<T, F> begin;
        SourceLocation<T, F> end;
        /** @brief Creates a source range from two source locations.
         *
         * @param begin Starting location of the range.
         * @param end Ending location of the range.
         */
        explicit SourceRange(SourceLocation<T, F> begin,
                             SourceLocation<T, F> end) :
            begin(begin), end(end) {
        }

        SourceRange(SourceRange&&)      = default;
        SourceRange(const SourceRange&) = default;
    };

    template<typename T>
    class SourcePositionRange {
        using offset_type = T;

       public:
        const offset_type begin;
        const offset_type end;
        explicit SourcePositionRange(offset_type begin, offset_type end) :
            begin(begin), end(end) {
        }

        template<typename F>
        SourceRange<offset_type, F> attach_file(F file) {
            return SourceRange<offset_type, F>(
                SourceLocation<offset_type, F>(begin, file),
                SourceLocation<offset_type, F>(begin, file));
        }
    };
}  // namespace Z::Zaban
