#pragma once
#include <cassert>

namespace util {

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
		if (ctr == 0) { return; } // no valid code found
		type = static_cast<int>(extraction);
		id = code % (10 * ctr);
	}
	[[nodiscard]] constexpr auto get_type() const -> int { return type; }
	[[nodiscard]] constexpr auto get_id() const -> int { return id; }
	[[nodiscard]] constexpr auto reveal_item() const -> bool { return type == 2; }
	[[nodiscard]] constexpr auto progress_quest() const -> bool { return type == 3; }

  private:
	int code{};
	int type{};
	int id{};
};

} // namespace util