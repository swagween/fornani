
#pragma once

#include <fornani/automa/MenuState.hpp>
#include <fornani/core/Fwd.hpp>
#include <fornani/graphics/Animatable.hpp>
#include <fornani/gui/NumberDisplay.hpp>

namespace fornani::automa {

class StatSheet final : public MenuState {
  public:
	StatSheet(ServiceProvider& svc, player::Player& player, AppContext& ctx);
	void tick_update(ServiceProvider& svc, capo::IEngine& engine) override;
	void frame_update(ServiceProvider& svc) override;
	void render(ServiceProvider& svc, sf::RenderWindow& win) override;

  private:
	Animatable m_items_sprite;
	Animatable m_guns_sprite;
	player::Player* m_player;
	sf::Text title;
	sf::Text stats;
	sf::Font stat_font{};
	util::Cooldown loading{2};
	std::vector<gui::NumberDisplay> m_number_displays{};
};

} // namespace fornani::automa
