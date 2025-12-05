#pragma once

#include "fornani/automa/MenuState.hpp"

namespace fornani::automa {

enum class SettingsToggles { autosprint, tutorial, gamepad, music, sfx, fullscreen, military_time };
enum class SettingsMenuMode { ready, adjust };

class SettingsMenu final : public MenuState {
  public:
	SettingsMenu(ServiceProvider& svc, player::Player& player);
	void tick_update(ServiceProvider& svc, capo::IEngine& engine) override;
	void frame_update(ServiceProvider& svc) override;
	void render(ServiceProvider& svc, sf::RenderWindow& win) override;

	[[nodiscard]] auto adjust_mode() const -> bool { return m_mode == SettingsMenuMode::adjust; }

  private:
	[[nodiscard]] auto is(SettingsToggles toggle) const -> bool { return static_cast<SettingsToggles>(current_selection.get()) == toggle; }
	util::BitFlags<SettingsToggles> toggles{};
	SettingsMenuMode m_mode{};
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
		sf::Text sfx_volume;
	} sliders;

	sf::Text music_label;
	sf::Text sfx_label;
};

} // namespace fornani::automa
