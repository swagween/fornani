
#pragma once

#include <algorithm>
#include <cassert>
#include <vector>

namespace fornani {

template <typename T>
class Register {
  public:
	using value_type = T;
	using container_type = std::vector<value_type>;
	using iterator = typename container_type::iterator;
	using const_iterator = typename container_type::const_iterator;

	bool add(T const& value) {
		if (!contains(value)) {
			items.push_back(value);
			return true;
		}
		return false;
	}

	bool add(T&& value) {
		if (!contains(value)) {
			items.push_back(std::move(value));
			return true;
		}
		return false;
	}

	bool remove(T const& value) {
		auto it = std::find(items.begin(), items.end(), value);
		if (it != items.end()) {
			items.erase(it);
			return true;
		}
		return false;
	}

	void clear() { items.clear(); }

	friend bool operator==(Register const& a, Register const& b) {
		if (a.items.size() != b.items.size()) return false;
		for (std::size_t i = 0; i < a.items.size(); ++i)
			if (a.items[i] != b.items[i]) return false;
		return true;
	}

	friend bool operator!=(Register const& a, Register const& b) { return !(a == b); }

	[[nodiscard]] auto contains(T const& value) const -> bool { return std::find(items.begin(), items.end(), value) != items.end(); }
	[[nodiscard]] auto is_empty() const -> bool { return items.empty(); }
	[[nodiscard]] auto size() const -> std::size_t { return items.size(); }

	iterator begin() { return items.begin(); }
	iterator end() { return items.end(); }

	const_iterator begin() const { return items.begin(); }
	const_iterator end() const { return items.end(); }

	const_iterator cbegin() const { return items.cbegin(); }
	const_iterator cend() const { return items.cend(); }

	T& operator[](size_t index) { return items[index]; }
	T const& operator[](size_t index) const { return items[index]; }

  private:
	container_type items;
};

} // namespace fornani
