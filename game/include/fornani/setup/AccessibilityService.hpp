#pragma once

namespace fornani::config {

class AccessibilityService {
  public:
	void set_action_ctx_bar_enabled(bool enabled) { m_action_ctx_bar_enabled = enabled; };

	[[nodiscard]] auto is_action_ctx_bar_enabled() const -> bool { return m_action_ctx_bar_enabled; };

  private:
	bool m_action_ctx_bar_enabled{};
};
} // namespace fornani::config