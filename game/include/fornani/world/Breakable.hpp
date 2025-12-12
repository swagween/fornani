
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/entities/packages/Health.hpp>
#include <fornani/graphics/Animatable.hpp>
#include <fornani/physics/RegisteredCollider.hpp>

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

class Map;

enum class BreakableAttributes { bulletproof };

class Breakable : public Animatable {
  public:
	Breakable(automa::ServiceProvider& svc, Map& map, sf::Vector2f position);
	void update(automa::ServiceProvider& svc, Map& map, player::Player& player);
	void handle_collision(shape::Collider& other) const;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);
	void on_hit(automa::ServiceProvider& svc, Map& map, arms::Projectile& proj, int power = 1);
	void on_smash(automa::ServiceProvider& svc, Map& map, int power = 1);
	void destroy() { m_health.kill(); }

	shape::Shape& get_bounding_box() { return m_collider.get()->bounding_box; }
	shape::Shape& get_hurtbox() { return m_collider.get()->hurtbox; }
	shape::Collider& get_collider() { return *m_collider.get(); }

	[[nodiscard]] auto is_destroyed() const -> bool { return m_health.is_dead(); }

  private:
	shape::RegisteredCollider m_collider;
	util::BitFlags<BreakableAttributes> attributes{};
	entity::Health m_health;
	float energy{};
	float dampen{0.1f};
	float hit_energy{8.f};
	sf::Vector2f random_offset{};
};

} // namespace fornani::world
