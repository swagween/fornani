
#pragma once

#include <array>
#include <cstdio>
#include <list>
#include <memory>
#include <unordered_map>
#include "Arsenal.hpp"
#include "../utils/Circuit.hpp"

namespace arms {

class Hotbar {
  public:
	Hotbar(int size);
	void add(int id);
	void remove(int id);
	void switch_weapon(automa::ServiceProvider& svc, int next);
	void set_selection(int id);
	bool has(int id) const;
	[[nodiscard]] auto get_selection() const -> int { return selection.get(); }
	[[nodiscard]] auto get_id() const -> int { return ids.at(selection.get()); }
	[[nodiscard]] auto get_id(int i) const -> int { return ids.at(i); }
	[[nodiscard]] auto size() const -> size_t { return ids.size(); }

	//for debug
	[[nodiscard]] auto get_ids() -> std::vector<int>& { return ids; }

  private:
	util::Circuit selection{1};
	std::vector<int> ids{};
	int max_size{3};
};

} // namespace arms