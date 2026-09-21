
#pragma once

#include <fornani/gui/hud/HUDWidget.hpp>

namespace fornani::gui {

class HUDItem final : public HUDWidget {
  public:
	HUDItem(automa::ServiceProvider& svc, player::Player& player, std::string_view tag, sf::Vector2f root, sf::Vector2i dimensions);

	void update(automa::ServiceProvider& svc, player::Player& player) override;
	void render(automa::ServiceProvider& svc, player::Player& player, sf::RenderWindow& win, sf::Vector2f offset = {}) override;

	private:
	sf::Vector2f get_spot(int i, sf::Vector2f offset) const;

  private:
	Animatable m_item_sprite;
};

} // namespace fornani::gui
