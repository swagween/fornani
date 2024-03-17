#pragma once
#include <SFML/Graphics.hpp>
#include "../Entity.hpp"

namespace entity {

class Health : public Entity {
	float const default_max{8.f};
  public:
	void set_max(float max);
	void update(automa::ServiceProvider& svc, world::Map& map);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	[[nodiscard]] auto get_hp() const -> float { return hp; }
	[[nodiscard]] auto is_dead() const -> bool { return hp <= 0.f; }
	void inflict(float amount);
  private:
	float max_hp{default_max};
	float hp{default_max};
};

} // namespace entity