
#pragma once

#include <fornani/automa/MenuState.hpp>
#include <fornani/gui/MiniMenu.hpp>
#include <fornani/gui/Slider.hpp>
#include <fornani/utils/Cooldown.hpp>

namespace fornani::automa {

enum class SettingsToggles : std::uint8_t { autosprint, tutorial, gamepad, master, music, ambience, sfx, fullscreen, military_time, language };
enum class SettingsMenuMode : std::uint8_t { ready, adjust };

struct VolumeSetting {
	sf::Text label;
	gui::Slider slider;
};

struct VolumeSettings {
	VolumeSetting master;
	VolumeSetting music;
	VolumeSetting ambience;
	VolumeSetting sfx;
};

class SettingsMenu final : public MenuState {
  public:
	SettingsMenu(ServiceProvider& svc, player::Player& player, AppContext& ctx);
	void on_exit() override;
	void tick_update(ServiceProvider& svc, capo::IEngine& engine) override;
	void frame_update(ServiceProvider& svc) override;
	void render(ServiceProvider& svc, sf::RenderWindow& win) override;

	[[nodiscard]] auto adjust_mode() const -> bool { return m_mode == SettingsMenuMode::adjust; }

  private:
	void adjust_volume(Volume& volume, double const amount, audio::Soundboard& soundboard);

	[[nodiscard]] auto is(SettingsToggles toggle) const -> bool { return static_cast<SettingsToggles>(current_selection.get()) == toggle; }

  private:
	util::BitFlags<SettingsToggles> toggles{};
	SettingsMenuMode m_mode{};
	std::optional<std::string_view> m_lang{};
	std::optional<gui::MiniMenu> m_menu{};
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
		sf::Text language;
	} toggleables;

	VolumeSettings volume_settings;

	util::Cooldown m_adjustment;
};

} // namespace fornani::automa
