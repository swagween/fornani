#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <string_view>
#include "../setup/EnumLookups.hpp"
#include "../utils/Collider.hpp"
#include "../utils/Counter.hpp"
#include "../entities/animation/Animation.hpp"
#include "SwitchButton.hpp"

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

class Map;

enum class SwitchBlockState { full, top, bottom, empty };

class SwitchBlock {
  public:
	SwitchBlock(automa::ServiceProvider& svc, sf::Vector2<float> position, int button_id, int type);
	void update(automa::ServiceProvider& svc, Map& map, player::Player& player);
	void handle_collision(shape::Collider& other) const;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam, bool background = false);
	void on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj, int power = 1);
	void turn_off() { state = SwitchBlockState::empty; }
	void turn_on() { state = SwitchBlockState::full; }
	shape::Shape& get_bounding_box() { return collider.bounding_box; }
	shape::Shape& get_hurtbox() { return collider.hurtbox; }
	[[nodiscard]] auto get_id() const -> int { return button_id; }
	[[nodiscard]] auto switched() const -> bool { return state != previous_state; }
	[[nodiscard]] auto on() const -> bool { return state != SwitchBlockState::empty; }
	[[nodiscard]] auto off() const -> bool { return state == SwitchBlockState::empty; }
	shape::Collider collider{};

  private:
	sf::Sprite sprite{};
	SwitchType type{};
	SwitchBlockState state{};
	SwitchBlockState previous_state{};
	int button_id{};
};
} // namespace world