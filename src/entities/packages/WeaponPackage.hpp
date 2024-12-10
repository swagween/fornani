#pragma once
#include <SFML/Graphics.hpp>
#include "../Entity.hpp"
#include "../../weapon/Weapon.hpp"
#include "../../utils/Cycle.hpp"

namespace enemy {
class Enemy;
}

namespace entity {

class WeaponPackage : public Entity {
  public:
	WeaponPackage(automa::ServiceProvider& svc, std::string_view label, int id);
	void update(automa::ServiceProvider& svc, world::Map& map, enemy::Enemy& enemy);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	void shoot();
	void set_team(arms::Team team);
	[[nodiscard]] auto get() -> arms::Weapon& { return *weapon; }
	[[nodiscard]] auto barrel_point() -> sf::Vector2<float> { return weapon->get_barrel_point(); }
	[[nodiscard]] auto get_cooldown() const -> int { return weapon->get_cooldown(); }

	util::Cooldown clip_cooldown{};
	util::Cycle cycle{2};
	int clip_cooldown_time{256};
	sf::Vector2<float> barrel_offset{};

  private:
	std::unique_ptr<arms::Weapon> weapon;
};

} // namespace entity