#pragma once

#include <vector> // std::vector
#include <utility> // std::pair
#include <algorithm> // std::lower_bound
#include <functional> // std::less

// Based on public domain code from https://github.com/michaeljclark/vecmap/blob/master/src/vecmap.h

namespace Metrics {
    template <class Key, class T, class Compare = std::less<Key>>
    class vecmap : protected std::vector<std::pair<Key, T>>
    {
    public:
        typedef std::pair<Key, T> value_type;
        typedef std::vector<value_type> base_type;

    public:
        typename base_type::iterator begin() { return base_type::begin(); };
        typename base_type::iterator end() { return base_type::end(); };
        typename base_type::const_iterator cbegin() const { return base_type::cbegin(); };
        typename base_type::const_iterator cend() const { return base_type::cend(); };

        vecmap() = default;
        vecmap(const vecmap&) = default;
        vecmap(vecmap&&) = default;
        ~vecmap() = default;

        /// <summary>
        /// Constructs vecmap instance from initalizer list. Note that if two or more items have same key, only one will be preserved
        /// </summary>
        /// <param name="_items">List of key-value pairs</param>
        vecmap(std::initializer_list<value_type> _items)
        {
            base_type::reserve(_items.size());
            for (const auto& item : _items)
                base_type::push_back(item);
            auto c = Compare();
            std::sort(begin(), end(), [&c](const value_type& v1, const value_type& v2) { return c(v1.first, v2.first); });
            auto last = std::unique(begin(), end(), [&c](const value_type& v1, const value_type& v2) { return c(v1.first, v2.first) == 0; });
            base_type::erase(last, end());
        }

        T& operator[](const Key& key)
        {
            auto c = Compare();
            auto i = std::lower_bound(begin(), end(), key, [&c](const value_type& l, const Key& k) { return c(l.first, k); });
            if (i == end() || c(key, i->first)) {
                i = base_type::insert(i, std::make_pair<Key, T>(std::move(Key(key)), T()));
            }
            return i->second;
        }

        inline bool operator<(const vecmap<Key, T, Compare>& other) const
        {
            // Slicing const reference to use std::vector comparison
            const base_type& b1 = *this;
            const base_type& b2 = other;
            return b1 < b2;
        }

        inline bool operator==(const vecmap<Key, T, Compare>& other) const
        {
            // Slicing const reference to use std::vector comparison
            const base_type& b1 = *this;
            const base_type& b2 = other;
            return b1 == b2;
        }

        inline bool operator!=(const vecmap<Key, T, Compare>& other) const
        {
            return !(*this == other);
        }
    };
}
