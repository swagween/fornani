#pragma once
#include <SFML/Graphics.hpp>
#include <fornani/graphics/Animatable.hpp>
#include <fornani/world/SwitchButton.hpp>
#include "fornani/utils/Collider.hpp"

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

enum class SwitchBlockState : std::uint8_t { full, top, bottom, empty };

class SwitchBlock : public Animatable {
  public:
	SwitchBlock(automa::ServiceProvider& svc, sf::Vector2f position, int button_id, int type);
	void update(automa::ServiceProvider& svc, Map& map, player::Player& player);
	void handle_collision(shape::Collider& other) const;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam, bool background = false);
	void on_hit(automa::ServiceProvider& svc, Map& map, arms::Projectile& proj) const;
	void turn_off() { m_state = SwitchBlockState::empty; }
	void turn_on() { m_state = SwitchBlockState::full; }
	shape::Shape& get_bounding_box() { return collider.bounding_box; }
	shape::Shape& get_hurtbox() { return collider.hurtbox; }
	[[nodiscard]] auto get_id() const -> int { return m_button_id; }
	[[nodiscard]] auto switched() const -> bool { return m_state != m_previous_state; }
	[[nodiscard]] auto on() const -> bool { return m_state != SwitchBlockState::empty; }
	[[nodiscard]] auto off() const -> bool { return m_state == SwitchBlockState::empty; }
	shape::Collider collider{};

  private:
	SwitchType m_type{};
	SwitchBlockState m_state{};
	SwitchBlockState m_previous_state{};
	int m_button_id{};
};
} // namespace fornani::world
