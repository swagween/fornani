#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string_view>
#include "fornani/setup/EnumLookups.hpp"
#include "fornani/utils/Collider.hpp"
#include "fornani/utils/Cooldown.hpp"
#include "fornani/utils/Counter.hpp"
#include "fornani/entities/animation/Animation.hpp"

namespace automa {
struct ServiceProvider;
}

namespace player {
class Player;
}

namespace arms {
class Projectile;
}

namespace world {

enum class PushableAttributes { bulletproof };
enum class PushableState { moved, moving };

class Pushable {
  public:
	Pushable(automa::ServiceProvider& svc, sf::Vector2<float> position, int style = 0, int size = 1);
	void update(automa::ServiceProvider& svc, Map& map, player::Player& player);
	void handle_collision(shape::Collider& other) const;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	void on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj);
	void reset(automa::ServiceProvider& svc, world::Map& map);
	void set_moving() { state.set(PushableState::moving); }
	shape::Shape& get_bounding_box() { return collider.bounding_box; }
	shape::Shape& get_hurtbox() { return collider.hurtbox; }
	[[nodiscard]] auto unmoved() { return !state.test(PushableState::moved); }
	[[nodiscard]] auto is_moving() { return state.test(PushableState::moving); }
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
	sf::Vector2<float> snap{};
	sf::Vector2<float> random_offset{};
	sf::Vector2<float> sprite_offset{-1.f, 0.f};
	sf::Vector2<float> start_position{};
	util::Counter hit_count{};
	util::Cooldown weakened{64};
};
} // namespace world