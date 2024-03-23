#pragma once
#include <SFML/Graphics.hpp>
#include "../Entity.hpp"
#include "../../weapon/Weapon.hpp"

namespace entity {

class WeaponPackage : public Entity {
  public:
	void update(automa::ServiceProvider& svc, world::Map& map);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	void shoot();
	[[nodiscard]] auto get_cooldown() const -> int { return weapon.cooldown.get_cooldown(); }

  private:
	arms::Weapon weapon{};
};

} // namespace entity