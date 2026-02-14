
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/components/SteeringComponent.hpp>
#include <fornani/core/Common.hpp>
#include <fornani/graphics/Animatable.hpp>
#include <fornani/physics/Shape.hpp>
#include <vector>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::player {
class Player;
}

namespace fornani::world {
class Map;
}

namespace fornani::entity {

class FloatingPart {
  public:
	FloatingPart(sf::Texture const& tex, float force, float friction, sf::Vector2f offset = {}, int id = 0);
	FloatingPart(automa::ServiceProvider& svc, std::string_view label, sf::Vector2i dimensions, std::vector<anim::Parameters> params, std::vector<std::string_view> labels, float force, float friction, sf::Vector2f offset = {}, int id = 0);
	FloatingPart(sf::Color color, sf::Vector2f dimensions, float force, float friction, sf::Vector2f offset = {}, int id = 0);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player, Direction direction, sf::Vector2f scale, sf::Vector2f position);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);
	void set_position(sf::Vector2f pos) { m_steering.physics.position = pos; }
	void set_force(float force) { m_attraction_force = force; }
	void set_shield(sf::Vector2f dim = {}, sf::Vector2f pos = {});
	void set_hitbox(sf::Vector2f dim = {}, sf::Vector2f pos = {});
	void set_magnitude(float magnitude);
	void move(sf::Vector2f distance);
	void apply_force(sf::Vector2f force) { m_steering.physics.apply_force(force); }
	void set_team(arms::Team to) { m_team = to; }

	[[nodiscard]] auto get_position() const -> sf::Vector2f { return m_steering.physics.position; }
	[[nodiscard]] auto get_center() const -> sf::Vector2f { return m_steering.physics.position; }
	[[nodiscard]] auto get_velocity() const -> sf::Vector2f { return m_steering.physics.velocity; }
	[[nodiscard]] auto get_id() const -> int { return m_id; }

	std::optional<sf::Sprite> sprite{};
	std::optional<Animatable> animated_sprite{};

  private:
	int m_id{};
	arms::Team m_team{arms::Team::skycorps}; // default to enemy team
	sf::Vector2f left{};
	sf::Vector2f right{};
	sf::Vector2f actual{};
	struct {
		float time{};
		float rate{0.03f};
		float magnitude{4.f};
	} m_movement{};
	bool init{};
	bool textured{};
	float m_attraction_force{0.01f};
	std::optional<sf::RectangleShape> drawbox{};
	std::optional<shape::Shape> hitbox{};
	std::optional<shape::Shape> shieldbox{};
	sf::RectangleShape debugbox{};
	components::SteeringComponent m_steering{};
};

} // namespace fornani::entity
