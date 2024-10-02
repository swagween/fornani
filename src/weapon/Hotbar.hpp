
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
	bool add(int id); // returns false if hotbar is full
	void remove(int id);
	void switch_weapon(automa::ServiceProvider& svc, int next);
	void set_selection(int id);
	[[nodiscard]] auto get_id() const -> int { return ids.at(selection.get()); }
	[[nodiscard]] auto size() const -> size_t { return ids.size(); }
  private:
	util::Circuit selection{1};
	std::vector<int> ids{};
	int max_size{3};
};

} // namespace arms
