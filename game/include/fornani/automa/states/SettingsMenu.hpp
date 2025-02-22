#pragma once

#include "fornani/automa/GameState.hpp"

namespace fornani::automa {

enum class Toggles : uint8_t { autosprint, tutorial, gamepad, music, fullscreen, military_time };
enum class MenuMode : uint8_t { ready, adjust };

class SettingsMenu final : public GameState {
  public:
	SettingsMenu(ServiceProvider& svc, player::Player& player, std::string_view scene = "", int room_number = 0);
	void tick_update(ServiceProvider& svc) override;
	void frame_update(ServiceProvider& svc) override;
	void render(ServiceProvider& svc, sf::RenderWindow& win) override;
	[[nodiscard]] auto adjust_mode() const -> bool { return m_mode == MenuMode::adjust; }

  private:
	util::BitFlags<Toggles> toggles{};
	MenuMode m_mode{};
	struct {
		sf::Text enabled;
		sf::Text disabled;
	} toggle_options;

	struct {
		sf::Text autosprint;
		sf::Text tutorial;
		sf::Text gamepad;
		sf::Text fullscreen;
		sf::Text military_time;
	} toggleables;

	struct {
		sf::Text music_volume;
	} sliders;

	sf::Text music_label;
};

} // namespace fornani::automa
