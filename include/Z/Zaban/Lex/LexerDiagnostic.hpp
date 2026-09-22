#pragma once

#include <Z/Zaban/SourcePosition.hpp>
#include <algorithm>
#include <vector>

namespace Z::Zaban::Lex {
    enum class LexerDiagnosticSeverity {
        Error,
        Warning,
        Deprecation,
        Info,
    };

    template<typename OffsetType   = std::size_t,
             typename SeverityType = LexerDiagnosticSeverity>
    class LexerDiagnostic {
       protected:
        OffsetRange<OffsetType> _range;
        SeverityType            _severity;

       public:
        LexerDiagnostic(OffsetRange<OffsetType> range, SeverityType severity) :
            _range(range), _severity(severity) {};

        OffsetRange<OffsetType> range() const noexcept {
            return this->_range;
        }

        SeverityType severity() const noexcept {
            return this->_severity;
        }
    };

    class LexerDiagnosticContextBase {
       public:
        virtual ~LexerDiagnosticContextBase() = default;

        virtual bool        has_errors() const noexcept        = 0;
        virtual std::size_t error_count() const noexcept       = 0;
        virtual std::size_t warning_count() const noexcept     = 0;
        virtual std::size_t deprecation_count() const noexcept = 0;
        virtual std::size_t info_count() const noexcept        = 0;
        virtual std::size_t scan_count() const                 = 0;
        virtual std::size_t concat_count() const               = 0;
        virtual void        record_scan() noexcept             = 0;
        virtual void        record_concatenation() noexcept    = 0;
    };

    template<typename DiagnosticType>
        requires std::derived_from<DiagnosticType, LexerDiagnostic<>>
    class LexerDiagnosticContext : public LexerDiagnosticContextBase {
       protected:
        std::vector<DiagnosticType> _diag_vector  = {};
        std::size_t                 _scan_count   = 0;
        std::size_t                 _concat_count = 0;

       public:
        virtual ~LexerDiagnosticContext() = default;

        virtual bool has_errors() const noexcept {
            return std::ranges::any_of(
                this->_diag_vector, [](const LexerDiagnostic<>& diagnostic) {
                    return diagnostic.severity() ==
                           LexerDiagnosticSeverity::Error;
                });
        }

        virtual std::size_t error_count() const noexcept {
            return std::count_if(this->_diag_vector.begin(),
                                 this->_diag_vector.end(),
                                 [](const LexerDiagnostic<>& diagnostic) {
                                     return diagnostic.severity() ==
                                            LexerDiagnosticSeverity::Error;
                                 });
        }

        virtual std::size_t warning_count() const noexcept {
            return std::count_if(this->_diag_vector.begin(),
                                 this->_diag_vector.end(),
                                 [](const LexerDiagnostic<>& diagnostic) {
                                     return diagnostic.severity() ==
                                            LexerDiagnosticSeverity::Warning;
                                 });
        }

        virtual std::size_t deprecation_count() const noexcept {
            return std::count_if(
                this->_diag_vector.begin(), this->_diag_vector.end(),
                [](const LexerDiagnostic<>& diagnostic) {
                    return diagnostic.severity() ==
                           LexerDiagnosticSeverity::Deprecation;
                });
        }

        virtual std::size_t info_count() const noexcept {
            return std::count_if(this->_diag_vector.begin(),
                                 this->_diag_vector.end(),
                                 [](const LexerDiagnostic<>& diagnostic) {
                                     return diagnostic.severity() ==
                                            LexerDiagnosticSeverity::Info;
                                 });
        }

        virtual std::size_t scan_count() const {
            return this->_scan_count;
        }

        virtual std::size_t concat_count() const {
            return this->_concat_count;
        }

        virtual void add(DiagnosticType diagnostic) {
            this->_diag_vector.push_back(std::move(diagnostic));
        }

        /// Folds another context into this one. chunks are lexed independently
        /// so everything a chunk records needs to survive the concat
        virtual void merge_from(const LexerDiagnosticContext& other) {
            this->_diag_vector.insert(this->_diag_vector.end(),
                                      other._diag_vector.begin(),
                                      other._diag_vector.end());

            this->_scan_count += other._scan_count;
            this->_concat_count += other._concat_count;
        }

        /// Drops recorded diagnostics but keeps the counters.
        virtual void clear_diagnostics() noexcept {
            this->_diag_vector.clear();
        }

        virtual void record_scan() noexcept {
            ++this->_scan_count;
        }

        virtual void record_concatenation() noexcept {
            ++this->_concat_count;
        }
    };
}  // namespace Z::Zaban::Lex
