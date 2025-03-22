
#pragma once

#include "fornani/gui/Console.hpp"
#include "fornani/gui/MiniMenu.hpp"

#include <optional>

namespace fornani::player {
class Player;
}

namespace fornani::automa {
class GameState;
}

namespace fornani::gui {

enum class PauseWindowState { active, settings, controls, exit, quit };

class PauseWindow {
  public:
	PauseWindow(automa::ServiceProvider& svc);
	void update(automa::ServiceProvider& svc, std::optional<std::unique_ptr<Console>>& console);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win);
	void reset();
	[[nodiscard]] auto settings_requested() const -> bool { return m_state == PauseWindowState::settings; }
	[[nodiscard]] auto controls_requested() const -> bool { return m_state == PauseWindowState::controls; }
	[[nodiscard]] auto exit_requested() const -> bool { return m_state == PauseWindowState::exit; }

  private:
	MiniMenu m_menu;
	sf::Vector2f m_dimensions{};
	sf::RectangleShape m_background{};
	PauseWindowState m_state{};
};

} // namespace fornani::gui
