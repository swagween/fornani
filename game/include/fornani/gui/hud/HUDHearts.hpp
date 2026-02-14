
#pragma once

#include <fornani/gui/hud/HUDWidget.hpp>

namespace fornani::gui {

class HUDHearts final : public HUDWidget {
  public:
	HUDHearts(automa::ServiceProvider& svc, player::Player& player, sf::Vector2f root, sf::Vector2f dimensions);

	void update(automa::ServiceProvider& svc, player::Player& player) override;
	void render(automa::ServiceProvider& svc, player::Player& player, sf::RenderWindow& win, sf::Vector2f offset = {}) override;

  private:
	WidgetBar m_health_bar;
};

} // namespace fornani::gui
