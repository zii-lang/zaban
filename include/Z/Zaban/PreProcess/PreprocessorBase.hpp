#pragma once

#include <Z/Zaban/PreProcess/IPreprocessor.hpp>

namespace Z::Zaban::Pp {
    /** @brief Identity preprocessor.
     *
     * Returns its input unchanged. Languages without a preprocessor (Python,
     * JS) use this directly, so the driver runs one code path for every
     * language. Languages with one (C, Z) override process().
     */
    template<typename T>
    class PreprocessorBase : public IPreprocessor<T> {
       public:
        std::vector<T> process(std::vector<T> tokens) override {
            return tokens;
        }
    };
}  // namespace Z::Zaban::Pp
