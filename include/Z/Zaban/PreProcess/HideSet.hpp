#pragma once

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace Z::Zaban::Pp {
    using HideSetId = std::uint32_t;

    /** @brief Interned sets of macro names.
     *
     * A token's hide set holds the macros already expanded to produce it. A
     * macro whose name is in its own hide set is not expanded again
     *
     * Most tokens in an expansion share a set, so sets are stored once and
     * referenced by id. Index 0 is always the empty set.
     */
    class HideSetTable {
       public:
        static constexpr HideSetId Empty = 0;

        HideSetTable() {
            _sets.emplace_back();
            _index.emplace(std::set<std::string>(), Empty);
        }

        bool contains(HideSetId id, const std::string& name) const {
            return _sets[id].contains(name);
        }

        HideSetId add(HideSetId id, const std::string& name) {
            if (this->contains(id, name)) return id;
            std::set<std::string> next = _sets[id];
            next.insert(name);
            return this->intern(std::move(next));
        }

        HideSetId merge(HideSetId a, HideSetId b) {
            if (a == Empty) return b;
            if (b == Empty) return a;
            if (a == b) return a;
            std::set<std::string> next = _sets[a];
            next.insert(_sets[b].begin(), _sets[b].end());
            return this->intern(std::move(next));
        }

        HideSetId intersect(HideSetId a, HideSetId b) {
            if (a == Empty || b == Empty) return Empty;
            if (a == b) return a;
            std::set<std::string> next;
            for (const auto& n: _sets[a]) {
                if (_sets[b].contains(n)) next.insert(n);
            }
            return this->intern(std::move(next));
        }

       private:
        HideSetId intern(std::set<std::string> s) {
            const auto it = _index.find(s);
            if (it != _index.end()) return it->second;

            const HideSetId id = static_cast<HideSetId>(_sets.size());
            _index.emplace(s, id);
            _sets.push_back(std::move(s));
            return id;
        }

        std::vector<std::set<std::string>>         _sets;
        std::map<std::set<std::string>, HideSetId> _index;
    };
}  // namespace Z::Zaban::Pp
