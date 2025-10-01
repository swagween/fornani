
#pragma once

namespace fornani {

template <typename T>
struct StateMachine {

	[[nodiscard]] auto is_state(T test) const -> bool { return p_state.actual == test; }
	[[nodiscard]] auto was_requested(T test) const -> bool { return p_state.desired == test; }

  protected:
	void request(T to) { p_state.desired = to; }
	struct {
		T actual{};
		T desired{};
	} p_state{};
};

} // namespace fornani
