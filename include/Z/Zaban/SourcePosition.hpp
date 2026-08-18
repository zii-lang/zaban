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
     * @tparam O Type used to represent the source offset.
     * @tparam F Type used to reference the source file.
     */
    template<typename O, typename F>
    class SourceLocation {
        using OffsetType     = O;
        using SourceFileType = F;

       public:
        const OffsetType     offset;
        const SourceFileType file_ref;
        /**
         * @brief Creates a source location.
         *
         * @param offset Offset within the source file.
         * @param file_ref Reference identifying the source file.
         */
        explicit SourceLocation(OffsetType offset, SourceFileType file_ref) :
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
     * @tparam O Type used to represent the source offset.
     * @tparam F Type used to reference the source file.
     */
    template<typename O, typename F>
    struct SourceRange {
       public:
        SourceLocation<O, F> begin;
        SourceLocation<O, F> end;
        /** @brief Creates a source range from two source locations.
         *
         * @param begin Starting location of the range.
         * @param end Ending location of the range.
         */
        explicit SourceRange(SourceLocation<O, F> begin,
                             SourceLocation<O, F> end) :
            begin(begin), end(end) {
        }

        SourceRange(SourceRange&&)      = default;
        SourceRange(const SourceRange&) = default;
    };

    template<typename OffsetType>
    class SourcePositionRange {
       public:
        OffsetType begin;
        OffsetType end;
        explicit SourcePositionRange(OffsetType begin, OffsetType end) :
            begin(begin), end(end) {
        }

        template<typename FileType>
        SourceRange<OffsetType, FileType> attach_file(FileType file) {
            return SourceRange<OffsetType, FileType>(
                SourceLocation<OffsetType, FileType>(begin, file),
                SourceLocation<OffsetType, FileType>(begin, file));
        }
    };
}  // namespace Z::Zaban
