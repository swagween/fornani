
#pragma once

#include "fornani/automa/MenuState.hpp"
#include "fornani/utils/Circuit.hpp"

namespace fornani::automa {

class ControlsMenu final : public MenuState {
  public:
	ControlsMenu(ServiceProvider& svc, player::Player& player);
	void tick_update(ServiceProvider& svc, capo::IEngine& engine) override;
	void frame_update(ServiceProvider& svc) override;
	void render(ServiceProvider& svc, sf::RenderWindow& win) override;
	void refresh_controls(ServiceProvider& svc);
	void restore_defaults(ServiceProvider& svc);

  private:
	void change_scene(ServiceProvider& svc, std::string_view scene); // FIXME this is a hack to work with GameState; eliminate options from GameState

	std::string m_scene{};
	float center_offset{320.f};
	std::vector<sf::Text> control_list{};
	util::Cooldown loading{};
	sf::Text instruction;
	bool option_is_selected{};
	bool binding_mode{};
	sf::RectangleShape debug{};
	util::Circuit m_current_tab;
};

} // namespace fornani::automa
