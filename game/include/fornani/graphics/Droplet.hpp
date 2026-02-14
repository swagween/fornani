#pragma once

#include "fornani/physics/CircleCollider.hpp"
#include "fornani/physics/Collider.hpp"
#include "fornani/utils/Cooldown.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::world {
class Map;
}

namespace fornani::vfx {
struct DropParams {
	float slant{};
	float sway{};
	float fall_speed{};
};
class Droplet {
  public:
	Droplet(sf::Vector2f start, DropParams params, float tweak);
	void update(automa::ServiceProvider& svc, world::Map& map, bool collision = true);
	void decay();
	[[nodiscard]] auto decayed() const -> bool { return post_collision.is_almost_complete(); }
	[[nodiscard]] auto collided() const -> bool { return collider.collided(); }
	[[nodiscard]] auto position() const -> sf::Vector2f { return collider.position(); }
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
