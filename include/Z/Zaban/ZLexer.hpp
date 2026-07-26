#pragma once

#include <string_view>

namespace Z::Zaban {
    class ZLexer {
       private:
       public:
        explicit ZLexer();
        ~ZLexer();

        ZLexer(const ZLexer&)            = delete;
        ZLexer& operator=(const ZLexer&) = delete;
        ZLexer(ZLexer&&);
        ZLexer& operator=(ZLexer&&);
    };
}  // namespace Z::Zaban
