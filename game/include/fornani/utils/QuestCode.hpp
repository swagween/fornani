#pragma once
#include <cassert>
#include <iostream>
#include <cmath>

namespace fornani::util {

	struct QuestKey {
	int type{};
	int id{};
	int source_id{};
	int amount{1};
	int hard_set{};
	friend bool operator==(QuestKey const& lhs, QuestKey const& rhs) { return lhs.type == rhs.type && lhs.id == rhs.id && lhs.source_id == rhs.source_id; }
	};

class QuestCode {
  public:
	constexpr QuestCode(int code) : code(code) {
		if (code < 1) { return; }
		auto extraction = static_cast<float>(code);
		auto ctr(0);
		while (extraction > 10.f) {
			extraction *= 0.1f;
			++ctr;
		}
		key.type = static_cast<int>(extraction);
		if (ctr == 0) { return; } // no valid code found
		key.id = code % (static_cast<int>(std::pow(10, ctr)));
	}
	[[nodiscard]] constexpr auto get_type() const -> int { return key.type; }
	[[nodiscard]] constexpr auto get_id() const -> int { return key.id; }
	[[nodiscard]] constexpr auto destroy_inspectable() const -> bool { return key.type == 1; }
	[[nodiscard]] constexpr auto reveal_item() const -> bool { return key.type == 2; }
	[[nodiscard]] constexpr auto progress_quest() const -> bool { return key.type == 3; }
	[[nodiscard]] constexpr auto retry() const -> bool { return key.type == 4; }

	QuestKey key{};

  private:
	int code{};
};

} // namespace util