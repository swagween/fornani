
#pragma once

#include <fornani/gui/hud/HUDWidget.hpp>

namespace fornani::gui {

class HUDOrbs final : public HUDWidget {
  public:
	HUDOrbs(automa::ServiceProvider& svc, player::Player& player, std::string_view tag, sf::Vector2f root, sf::Vector2i dimensions);

	void update(automa::ServiceProvider& svc, player::Player& player) override;
	void render(automa::ServiceProvider& svc, player::Player& player, sf::RenderWindow& win, sf::Vector2f offset = {}) override;

  private:
	Drawable m_orb_text;
	std::string m_digits{};
	float m_label_width;
};

} // namespace fornani::gui
