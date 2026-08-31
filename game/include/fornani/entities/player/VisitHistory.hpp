
#pragma once

#include <fornani/systems/Register.hpp>
#include <deque>
#include <vector>

namespace fornani::player {

constexpr int far_distance_v{8};

class VisitHistory {
  public:
	void push_room(int const id);
	void clear() { room_deque.clear(); }

	[[nodiscard]] auto distance_traveled_from(int const room_id) const -> int;
	[[nodiscard]] auto traveled_far_from(int const room) const -> bool { return distance_traveled_from(room) > far_distance_v; }
	[[nodiscard]] auto collapsed_path() const -> std::vector<int>;
	[[nodiscard]] auto get_list() const -> std::vector<int>;
	[[nodiscard]] auto has_visited(int const room_id) const -> bool;

  private:
	std::deque<int> room_deque{};
};

} // namespace fornani::player
