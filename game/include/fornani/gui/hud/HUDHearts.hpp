
#pragma once

#include <fornani/gui/hud/HUDWidget.hpp>

namespace fornani::gui {

class HUDHearts final : public HUDWidget {
  public:
	HUDHearts(automa::ServiceProvider& svc, player::Player& player, sf::Vector2f root, sf::Vector2f dimensions);

	void update(automa::ServiceProvider& svc, player::Player& player) override;
	void render(automa::ServiceProvider& svc, player::Player& player, sf::RenderWindow& win, sf::Vector2f offset = {}) override;
	void refresh(automa::ServiceProvider& svc, player::Player& player);

	[[nodiscard]] auto get_endpoint() const -> sf::Vector2f { return m_health_bar ? m_health_bar->get_endpoint() : sf::Vector2f{}; }

  private:
	std::optional<WidgetBar> m_health_bar{};
	sf::Vector2f m_dimensions;
};

} // namespace fornani::gui
