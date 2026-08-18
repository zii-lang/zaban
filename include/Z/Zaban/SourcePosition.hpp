#pragma once

#include <Z/Zaban/Config.hpp>
#include <cstddef>
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
    struct SourceLocation {
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

    /**
     * @brief Represents a half-open range of source offsets.
     *
     * The range is defined by a beginning offset and an ending offset, where
     * @p begin is inclusive and @p end is exclusive.
     *
     * @tparam OffsetType The type used to represent source offsets.
     *
     * @deprecated Use OffsetRange instead.
     */
    template<typename OffsetType>
    struct ZABAN_DEPRECATED("Use OffsetRange instead.") SourcePositionRange {
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
                SourceLocation<OffsetType, FileType>(end, file));
        }
    };

    /**
     * @brief Represents a half-open range of source offsets.
     *
     * An OffsetRange describes a contiguous range using an inclusive beginning
     * offset and an exclusive ending offset. It is independent of any
     * particular source file and can be associated with one using
     * attach_file().
     *
     * @tparam OffsetType The type used to represent source offsets.
     */
    template<typename OffsetType = std::size_t>
    struct OffsetRange {
       public:
        OffsetType begin;
        OffsetType end;

        /**
         * @brief Constructs an offset range.
         *
         * @param begin The inclusive beginning offset.
         * @param end The exclusive ending offset.
         */
        OffsetRange(OffsetType begin, OffsetType end) :
            begin(begin), end(end) {};

        /**
         * @brief Associates this range with a source file.
         *
         * @tparam FileType The type used to identify the source file.
         * @param file The source file to associate with the range.
         * @return A SourceRange containing the beginning and ending locations.
         */
        template<typename FileType>
        SourceRange<OffsetType, FileType> attach_file(FileType file) {
            return SourceRange<OffsetType, FileType>(
                SourceLocation<OffsetType, FileType>(begin, file),
                SourceLocation<OffsetType, FileType>(end, file));
        }
    };

    /**
     * @brief Calculates the length of an offset range.
     *
     * The range follows the half-open convention [begin, end), so the length
     * is calculated as end - begin.
     *
     * @tparam OffsetType The type used to represent source offsets.
     * @param range The offset range whose length is calculated.
     * @return The number of offsets contained in the range.
     */
    template<typename OffsetType>
    constexpr OffsetType length(const OffsetRange<OffsetType>& range) {
        if (range.end < range.begin) Z_UNLIKELY {
                return 0;
            }
        return range.end - range.begin;
    }
}  // namespace Z::Zaban
