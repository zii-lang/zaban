#include <Z/Zaban/Langs/CLang/Lexer.hpp>
#include <Z/Zaban/Langs/CLang/Preprocessor.hpp>
#include <Z/Zaban/Langs/CLang/TokenKind.hpp>
#include <filesystem>
#include <fstream>
#include <iterator>

namespace Z::Zaban::Langs::CLang {
    namespace {
        class DiskInclude : public IncludeSource {
           public:
            bool read(const std::string& path, std::string& out) override {
                std::ifstream f(path, std::ios::binary);
                if (!f) return false;
                out.assign(std::istreambuf_iterator<char>(f),
                           std::istreambuf_iterator<char>());
                return true;
            }
        };
    }  // namespace

    IncludeSource& disk_include_source() {
        static DiskInclude reader;
        return reader;
    }

    bool CPreprocessor::read_header_name(const std::vector<PpToken>& line,
                                         std::string&                path,
                                         bool& angled) const {
        if (line.empty()) return false;

        if (CLexerTokenKind::String == line[0].token.kind) {
            const std::string s = this->spelling(line[0].token);
            if (s.size() < 2 || '"' != s.front() || '"' != s.back()) {
                return false;
            }
            path   = s.substr(1, s.size() - 2);
            angled = false;
            return true;
        }

        if (CLexerTokenKind::Lesser != line[0].token.kind) return false;

        /* `<stdio.h>` lexes as Lesser Identifier Dot Identifier Greater, and
           `<sys/types.h>` adds a Slash. Rather than add a header-name kind
           to the lexer, read the bytes back: everything between the brackets
           is contiguous in whichever buffer it came from.
         */
        std::size_t j = 1;
        while (j < line.size() &&
               CLexerTokenKind::Greater != line[j].token.kind) {
            ++j;
        }
        if (j >= line.size() || 1 == j) return false;

        path   = std::string(_sources.text(line[1].token.range.begin,
                                           line[j - 1].token.range.end));
        angled = true;
        return true;
    }

    bool CPreprocessor::find_header(const std::string& name, bool angled,
                                    std::string& resolved,
                                    std::string& text) const {
        namespace fs = std::filesystem;

        std::vector<fs::path> candidates;

        // A quoted include looks next to the file that asked for it first.
        if (!angled && !_files.empty()) {
            candidates.push_back(fs::path(_files.back()).parent_path() / name);
        }
        for (const auto& dir: _include_dirs) {
            candidates.push_back(fs::path(dir) / name);
        }
        if (candidates.empty()) candidates.push_back(fs::path(name));

        for (const auto& c: candidates) {
            std::string s = c.lexically_normal().string();
            if (_included.contains(s)) {
                resolved = std::move(s);
                return true;
            }
            if (_reader && _reader->read(s, text)) {
                resolved = std::move(s);
                return true;
            }
        }
        return false;
    }

    void CPreprocessor::handle_include(std::vector<PpToken>& tokens,
                                       const Directive&      d,
                                       std::vector<PpToken>& out) {
        if (d.hash_index + 2 >= d.end_index) {
            _errors |= CPpErrorFlags::MalformedDirective;
            return;
        }

        std::vector<PpToken> line(tokens.begin() + d.hash_index + 2,
                                  tokens.begin() + d.end_index);

        std::string path;
        bool        angled = false;

        if (!this->read_header_name(line, path, angled)) {
            // `#include HDR` — expand the line, then read the result as a
            // header name.
            constexpr TokenFlags strip =
                TokenFlags::DirectiveLine | TokenFlags::AtLineStart;
            for (auto& t: line) {
                t.token.flags &= static_cast<std::uint16_t>(~strip);
            }

            std::vector<PpToken> expanded;
            for (std::size_t i = 0; i < line.size();) {
                i = this->expand_into(line, i, expanded);
            }

            if (!this->read_header_name(expanded, path, angled)) {
                _errors |= CPpErrorFlags::MalformedDirective;
                return;
            }
        }

        if (_files.size() >= MaxIncludeDepth) {
            _errors |= CPpErrorFlags::IncludeTooDeep;
            return;
        }

        std::string resolved;
        std::string text;
        if (!this->find_header(path, angled, resolved, text)) {
            _errors |= CPpErrorFlags::IncludeNotFound;
            return;
        }

        const auto [slot, fresh] = _included.try_emplace(resolved);
        if (!fresh && slot->second.once) return;
        if (fresh) {
            const auto       base = _sources.add(std::move(text), resolved);
            CLexerBufferType buf  = _sources.whole(base);
            CLexer           lx(buf, base);
            lx.scan();
            for (auto& t: lx.finalize()) {
                // header's eof marker would land mid strm
                if (CLexerTokenKind::Eob == t.kind) continue;
                slot->second.tokens.push_back(
                    PpToken{std::move(t), Pp::HideSetTable::Empty});
            }
            if (!slot->second.tokens.empty()) {
                slot->second.tokens.front().token.flags |=
                    static_cast<std::uint16_t>(TokenFlags::AtLineStart);
            }
        }

        std::vector<PpToken> in = slot->second.tokens;

        _files.push_back(resolved);
        const std::size_t depth = _cond.size();

        this->run(in, out);

        // A conditional has to be balanced inside the file that opened it.
        if (_cond.size() != depth) {
            _errors |= CPpErrorFlags::UnterminatedIf;
            _cond.resize(depth);
        }
        _files.pop_back();
    }

    void CPreprocessor::handle_pragma(const std::vector<PpToken>& tokens,
                                      const Directive&            d) {
        if (d.hash_index + 2 >= d.end_index) return;
        if (this->spelling(tokens[d.hash_index + 2].token) != "once") return;

        // _files.back is the file currently being run. the main file is not in
        // the cache so the 'once' in the main file wont have anny effect there
        const auto it = _included.find(_files.back());
        if (it != _included.end()) it->second.once = true;
    }
}  // namespace Z::Zaban::Langs::CLang
