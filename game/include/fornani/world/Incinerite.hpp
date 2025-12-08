
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/entities/packages/Health.hpp>
#include <fornani/graphics/Animatable.hpp>
#include <fornani/utils/Collider.hpp>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::player {
class Player;
}

namespace fornani::arms {
class Projectile;
}

namespace fornani::world {

class Incinerite : public Animatable {
  public:
	Incinerite(automa::ServiceProvider& svc, sf::Vector2f position, int chunk_id);
	void update(automa::ServiceProvider& svc, Map& map, player::Player& player);
	void handle_collision(shape::Collider& other) const;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);
	void on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj, int power = 1);
	void hit();

	shape::Shape& get_bounding_box() { return collider.bounding_box; }
	shape::Shape& get_hurtbox() { return collider.hurtbox; }
	[[nodiscard]] auto is_destroyed() const -> bool { return health.is_dead(); }
	[[nodiscard]] auto get_chunk_id() const -> int { return m_chunk_id; }

	entity::Health health{};

  private:
	shape::Collider collider{};
	int m_chunk_id{};
	float energy{};
	float dampen{0.1f};
	float hit_energy{8.f};
	sf::Vector2f random_offset{};
};
} // namespace fornani::world
