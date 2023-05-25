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

		vecmap(std::initializer_list<value_type> _items)
		{
			reserve(_items.size());
			for (const auto& item : _items)
				push_back(item);
			auto c = Compare();
			std::sort(begin(), end(), [&c](const value_type& v1, const value_type& v2) { return c(v1.first, v2.first); });
			if (std::adjacent_find(begin(), end(), [&c](const value_type& v1, const value_type& v2) { return c(v1.first, v2.first) == 0; }) != end())
				throw std::exception("Non-unique keys in arguments");
		}

		T& operator[](const Key& key)
		{
			auto c = Compare();
			auto i = std::lower_bound(begin(), end(), key, [&c](const value_type& l, const Key& k) { return c(l.first, k); });
			if (i == end() || c(key, i->first)) {
				i = insert(i, std::make_pair<Key,T>(std::move(Key(key)), T()));
			}
			return i->second;
		}

		template <class Key, class T, class Compare = std::less<Key>>
		inline bool operator<(const vecmap<Key, T, Compare>& other) const
		{
			const vecmap<Key, T, Compare>::base_type& b1 = *this;
			const vecmap<Key, T, Compare>::base_type& b2 = other;
			return b1 < b2;
		}

		template <class Key, class T, class Compare = std::less<Key>>
		inline bool operator==(const vecmap<Key, T, Compare>& other) const
		{
			const vecmap<Key, T, Compare>::base_type& b1 = *this;
			const vecmap<Key, T, Compare>::base_type& b2 = other;
			return b1 == b2;
		}

		template <class Key, class T, class Compare = std::less<Key>>
		inline bool operator!=(const vecmap<Key, T, Compare>& other) const
		{
			return !(*this == other);
		}
	};
}
