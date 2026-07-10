
#pragma once

#include <fornani/systems/Register.hpp>
#include <deque>
#include <vector>

namespace fornani::player {

class VisitHistory {
  public:
	[[nodiscard]] auto distance_traveled_from(int const room_id) const -> int {
		auto cache = std::vector<int>{};
		for (auto& room : room_deque) {
			if (room == room_id) { cache.clear(); }
			if (std::ranges::find(cache, room) == cache.end()) { cache.push_back(room); }
		}
		return static_cast<int>(cache.size());
	}
	[[nodiscard]]
	auto distance_traveled() const -> int {
		auto path = collapsed_path();
		return path.empty() ? 0 : static_cast<int>(path.size()) - 1;
	}
	[[nodiscard]] auto traveled_far() const -> bool { return distance_traveled() > far_distance; }
	void clear() { room_deque.clear(); }
	void push_room(int const id) {
		room_deque.push_back(id);
		if (room_deque.size() >= max_size) { room_deque.pop_front(); }
	}
	[[nodiscard]]
	auto collapsed_path() const -> std::vector<int> {
		std::vector<int> path;
		for (auto room : room_deque) {
			if (path.size() >= 2 && path[path.size() - 2] == room) {
				path.pop_back();
			} else {
				path.push_back(room);
			}
		}
		return path;
	}
	std::deque<int> room_deque{};

  private:
	int far_distance{8};
	std::size_t max_size{64};
};

} // namespace fornani::player
