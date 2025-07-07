#pragma once
#include <SFML/Graphics.hpp>
#include "fornani/utils/Collider.hpp"
#include "fornani/utils/Cooldown.hpp"
#include "fornani/utils/Counter.hpp"

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
enum class PushableState : std::uint8_t { moved, moving };

class Pushable {
  public:
	Pushable(automa::ServiceProvider& svc, sf::Vector2f position, int style = 0, int size = 1);
	void update(automa::ServiceProvider& svc, Map& map, player::Player& player);
	void handle_collision(shape::Collider& other) const;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);
	void on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj);
	void reset(automa::ServiceProvider& svc, world::Map& map);
	void set_moving() { state.set(PushableState::moving); }
	shape::Shape& get_bounding_box() { return collider.bounding_box; }
	shape::Shape& get_hurtbox() { return collider.hurtbox; }
	[[nodiscard]] auto unmoved() const { return !state.test(PushableState::moved); }
	[[nodiscard]] auto is_moving() const { return state.test(PushableState::moving); }
	shape::Collider collider{};
	shape::Shape start_box{};

  private:
	util::BitFlags<PushableAttributes> attributes{};
	util::BitFlags<PushableState> state{};
	sf::Sprite sprite;
	int style{};
	int size{};
	float mass{};
	float dampen{0.1f};
	float speed{64.f};
	float energy{};
	float hit_energy{8.f};
	sf::Vector2f snap{};
	sf::Vector2f random_offset{};
	sf::Vector2f sprite_offset{-1.f, 0.f};
	sf::Vector2f start_position{};
	util::Counter hit_count{};
	util::Cooldown weakened{64};
};
} // namespace fornani::world