
#pragma once

#include <optional>

namespace fornani {

template <typename T>
struct StateMachine {
	void set(T to) {
		p_state.actual = to;
		p_state.desired = to;
	}
	[[nodiscard]] auto is_state(T test) const -> bool { return p_state.actual == test; }
	[[nodiscard]] auto was_requested(T test) const -> bool { return p_state.desired == test; }
	void request(T to) { p_state.desired = to; }

  protected:
	struct {
		T actual{};
		T desired{};
	} p_state{};
};

} // namespace fornani
