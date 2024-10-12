
#pragma once
#include <vector>

namespace player {
class VisitHistory {
  public:
	[[nodiscard]] auto distance_traveled() const -> int { return static_cast<int>(rooms_visited.size()); }
	[[nodiscard]] auto traveled_far() const -> bool { return rooms_visited.size() > far_distance; }
	constexpr void clear() { rooms_visited.clear(); }
	constexpr void push_room(int id) {
		if (std::ranges::find(rooms_visited.begin(), rooms_visited.end(), id) == rooms_visited.end()) { rooms_visited.push_back(id); }
	}

	std::vector<int> rooms_visited{};
  private:
	int far_distance{6};
};
} // namespace player
