
#include <fornani/entities/player/VisitHistory.hpp>

namespace fornani::player {

constexpr std::size_t max_room_history_v{64};

void VisitHistory::push_room(int const id) {
	if (room_deque.empty() || room_deque.back() != id) { room_deque.push_back(id); }
	if (room_deque.size() > max_room_history_v) { room_deque.pop_front(); }
}

auto player::VisitHistory::distance_traveled_from(int const room_id) const -> int {
	auto cache = std::vector<int>{};
	for (auto const room : room_deque) {
		if (room == room_id) { cache.clear(); }
		if (std::ranges::find(cache, room) == cache.end()) { cache.push_back(room); }
	}
	return static_cast<int>(cache.size());
}

auto VisitHistory::collapsed_path() const -> std::vector<int> {
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

auto VisitHistory::get_list() const -> std::vector<int> {
	auto ret = std::vector<int>();
	return {room_deque.begin(), room_deque.end()};
}

auto VisitHistory::has_visited(int const room_id) const -> bool { return std::ranges::find(room_deque, room_id) != room_deque.end(); }

} // namespace fornani::player
