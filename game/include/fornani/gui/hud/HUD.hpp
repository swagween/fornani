
#pragma once

#include <fornani/gui/hud/HUDAmmo.hpp>
#include <fornani/gui/hud/HUDGun.hpp>
#include <fornani/gui/hud/HUDHearts.hpp>
#include <fornani/gui/hud/HUDOrbs.hpp>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::player {
class Player;
}

namespace fornani::gui {

class HUD final {

  public:
	HUD(automa::ServiceProvider& svc, player::Player& player);
	void update(automa::ServiceProvider& svc, player::Player& player);
	void render(automa::ServiceProvider& svc, player::Player& player, sf::RenderWindow& win);
	void set_position(sf::Vector2f const to) { m_position = to; };
	void reset_position() { m_position = m_origin; };

  private:
	sf::Vector2f m_origin{};
	sf::Vector2f m_position{};

	HUDHearts hearts;
	HUDOrbs orbs;
	HUDAmmo ammo;
	HUDGun gun;
};

} // namespace fornani::gui
