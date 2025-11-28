
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/graphics/Drawable.hpp>
#include <fornani/utils/Collider.hpp>
#include <fornani/utils/Cooldown.hpp>
#include <fornani/utils/Counter.hpp>

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

enum class PushableAttributes : std::uint8_t { bulletproof };
enum class PushableState : std::uint8_t { moved, pushed };

class Pushable : public Drawable {
  public:
	Pushable(automa::ServiceProvider& svc, sf::Vector2f position, int style = 0, int size = 1);
	void update(automa::ServiceProvider& svc, Map& map, player::Player& player);
	void post_update(automa::ServiceProvider& svc, Map& map, player::Player& player);
	void handle_collision(shape::Collider& other) const;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);
	void on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj);
	void reset(automa::ServiceProvider& svc, world::Map& map);
	void register_chunk(std::uint8_t chunk) { m_chunk_id = chunk; }

	shape::Shape& get_bounding_box() { return collider.bounding_box; }
	shape::Shape& get_hurtbox() { return collider.hurtbox; }

	[[nodiscard]] auto unmoved() const { return !state.test(PushableState::moved); }
	[[nodiscard]] auto is_being_pushed() const { return state.test(PushableState::pushed); }
	[[nodiscard]] auto is_moving() const { return collider.physics.position != collider.physics.previous_position; }
	[[nodiscard]] auto get_chunk_id() const -> std::uint8_t { return m_chunk_id; }
	[[nodiscard]] auto get_size() const -> int { return size; }

	shape::Collider collider{};
	shape::Shape start_box{};
	shape::Shape collision_box{};

  private:
	util::BitFlags<PushableAttributes> attributes{};
	util::BitFlags<PushableState> state{};

	int style{};
	int size{};
	float mass{};
	float dampen{0.1f};
	float speed{};
	float energy{};
	float hit_energy{8.f};

	sf::Vector2f snap{};
	sf::Vector2f random_offset{};
	sf::Vector2f start_position{};
	util::Counter hit_count{};
	util::Cooldown weakened{64};
	std::uint8_t m_chunk_id{};
};
} // namespace fornani::world
