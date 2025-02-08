
#pragma once
#include <vector>

namespace fornani::player {
class VisitHistory {
  public:
	[[nodiscard]] auto distance_traveled_from(int room_id) -> int {
		auto cache = std::vector<int>{};
		for (auto& room : room_deque) {
			if (room == room_id) { cache.clear(); }
			if (std::find(cache.begin(), cache.end(), room) == cache.end()) { cache.push_back(room); }
		}
		return static_cast<int>(cache.size());
	}
	[[nodiscard]] auto distance_traveled() const -> int { return static_cast<int>(rooms_visited.size()); }
	[[nodiscard]] auto traveled_far() const -> bool { return rooms_visited.size() > far_distance; }
	constexpr void clear() { rooms_visited.clear(); }
	void push_room(int id) {
		if (std::ranges::find(rooms_visited.begin(), rooms_visited.end(), id) == rooms_visited.end()) { rooms_visited.push_back(id); }
		room_deque.push_back(id);
		if (room_deque.size() >= max_size) { room_deque.pop_front(); }
	}

	std::vector<int> rooms_visited{};
	std::deque<int> room_deque{};
  private:
	int far_distance{6};
	size_t max_size{64};
};
} // namespace fornani::player
