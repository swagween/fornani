
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/graphics/Drawable.hpp>
#include <fornani/physics/RegisteredCollider.hpp>
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

class Map;

enum class PushableAttributes { bulletproof };
enum class PushableState { moved, pushed };

class Pushable : public Drawable {
  public:
	Pushable(automa::ServiceProvider& svc, Map& map, sf::Vector2f position, int style = 0, int size = 1);
	Pushable(Pushable&&) = delete;
	Pushable& operator=(Pushable&&) = delete;
	void update(automa::ServiceProvider& svc, Map& map, player::Player& player);
	void post_update(automa::ServiceProvider& svc, Map& map, player::Player& player);
	void handle_collision(shape::Collider& other) const;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);
	void on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj);
	void reset(automa::ServiceProvider& svc, world::Map& map);
	void register_chunk(std::uint8_t chunk) { m_chunk_id = chunk; }

	void set_push(bool const to) { to ? state.set(PushableState::pushed) : state.reset(PushableState::pushed); }

	shape::Shape& get_bounding_box() { return m_collider.get()->bounding_box; }
	shape::Shape& get_hurtbox() { return m_collider.get()->hurtbox; }
	shape::Collider& get_collider() { return *m_collider.get(); }

	[[nodiscard]] auto unmoved() const { return !state.test(PushableState::moved); }
	[[nodiscard]] auto is_being_pushed() const { return state.test(PushableState::pushed); }
	[[nodiscard]] auto is_moving() const { return m_collider.get()->physics.position != m_collider.get()->physics.previous_position; }
	[[nodiscard]] auto get_chunk_id() const -> std::uint8_t { return m_chunk_id; }
	[[nodiscard]] auto get_size() const -> int { return size; }

	shape::Shape start_box{};
	shape::Shape collision_box{};

  private:
	Map* m_map;
	shape::RegisteredCollider m_collider;

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
