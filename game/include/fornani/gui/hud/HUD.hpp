
#pragma once

#include <fornani/gui/hud/HUDAmmo.hpp>
#include <fornani/gui/hud/HUDGun.hpp>
#include <fornani/gui/hud/HUDHearts.hpp>
#include <fornani/gui/hud/HUDOrbs.hpp>
#include <fornani/particle/Effect.hpp>

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
	void refresh_hearts(automa::ServiceProvider& svc, player::Player& player) { hearts.refresh(svc, player); }
	void spawn_effect(automa::ServiceProvider& svc, std::string_view tag, sf::Vector2f pos, sf::Vector2f vel = {}, int channel = 0);

	[[nodiscard]] auto get_hearts_endpoint() const -> sf::Vector2f { return m_position + hearts.get_endpoint(); }

  private:
	sf::Vector2f m_origin{};
	sf::Vector2f m_position{};
	std::vector<entity::Effect> m_effects{};

	HUDHearts hearts;
	HUDOrbs orbs;
	HUDAmmo ammo;
	HUDGun gun;
};

} // namespace fornani::gui
