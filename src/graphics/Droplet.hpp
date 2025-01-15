#pragma once

#include <SFML/Graphics.hpp>
#include "../utils/CircleCollider.hpp"
#include "../utils/Collider.hpp"
#include <vector>

namespace automa {
struct ServiceProvider;
}

namespace world {
class Map;
}

namespace vfx {
struct DropParams {
	float slant{};
	float sway{};
	float fall_speed{};
};
class Droplet {
  public:
	Droplet(sf::Vector2<float> start, DropParams params, float tweak);
	void update(automa::ServiceProvider& svc, world::Map& map, bool collision = true);
	void decay();
	[[nodiscard]] auto decayed() const -> bool { return post_collision.is_almost_complete(); }
	[[nodiscard]] auto collided() const -> bool { return collider.collided(); }
	[[nodiscard]] auto position() const -> sf::Vector2<float> { return collider.position(); }
	[[nodiscard]] auto get_angle() const -> float { return angle; }
	shape::CircleCollider collider;
	float z{};
  private:
	util::Counter counter{};
	util::Cooldown post_collision{64};
	float angle{};
	DropParams params{};
};
} // namespace vfx
