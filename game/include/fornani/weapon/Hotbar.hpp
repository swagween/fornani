
#pragma once

#include "Arsenal.hpp"
#include "fornani/utils/Circuit.hpp"

namespace fornani::arms {

class Hotbar {
  public:
	explicit Hotbar(int size);
	void add(int id);
	void remove(int id);
	void switch_weapon(automa::ServiceProvider& svc, int next);
	void set_selection(int id);
	void sync();
	bool has(int id) const;
	[[nodiscard]] auto get_selection() const -> int { return selection.get(); }
	[[nodiscard]] auto get_id() const -> int { return ids.at(selection.get()); }
	[[nodiscard]] auto get_id(int i) const -> int { return ids.at(i); }
	[[nodiscard]] auto size() const -> std::size_t { return ids.size(); }
	[[nodiscard]] auto switched() const -> bool { return selection.get() != previous; }

	// for debug
	[[nodiscard]] auto get_ids() -> std::vector<int>& { return ids; }

  private:
	util::Circuit selection{1};
	std::vector<int> ids{};
	int max_size{3};
	int previous{-1};
};

} // namespace fornani::arms
