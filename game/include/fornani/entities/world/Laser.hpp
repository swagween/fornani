
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/core/Common.hpp>
#include <fornani/entity/Turret.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/physics/Shape.hpp>
#include <fornani/utils/Circuit.hpp>
#include <fornani/utils/Cooldown.hpp>
#include <fornani/utils/Direction.hpp>

namespace fornani {

namespace automa {
struct ServiceProvider;
}

namespace player {
class Player;
}

namespace world {

class Map;

enum class LaserType { turret, magic };
enum class LaserAttributes { transcendent, infinite, player };

class Laser {
  public:
	Laser(automa::ServiceProvider& svc, Map& map, sf::Vector2f position, LaserType type, util::BitFlags<LaserAttributes> attributes, CardinalDirection direction, int active = 128, int cooldown = 128, float size = 1.f);
	Laser(automa::ServiceProvider& svc, Map& map, Turret& parent, sf::Vector2f position, LaserType type, util::BitFlags<LaserAttributes> attributes, CardinalDirection direction, int active = 128, int cooldown = 128, float size = 1.f);
	void update(automa::ServiceProvider& svc, player::Player& player, Map& map);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);
	void fire() { m_active.start(); }

	[[nodiscard]] auto is(LaserType const test) const -> bool { return m_type == test; }
	[[nodiscard]] auto is_active() const -> bool { return m_active.running(); }
	[[nodiscard]] auto is_complete() const -> bool { return m_cooldown.is_almost_complete(); }

  private:
	std::optional<Turret*> m_parent{};
	sf::Vector2f calculate_size(Map& map);
	sf::Vector2f calculate_end_point();
	void handle_collision(shape::Shape& obstacle, sf::Vector2f size);
	std::optional<sf::Vector2f> calculate_collision_point(shape::Shape& other);

	LaserType m_type{};
	arms::Team m_team{};
	util::BitFlags<LaserAttributes> m_attributes{};
	CardinalDirection m_direction{};
	shape::Shape m_hitbox;
	sf::RectangleShape m_drawbox{};
	util::Cooldown m_active;
	util::Cooldown m_cooldown;
	util::Circuit m_pulse{2};
	sf::Vector2f m_spawn_point{};
	float m_size{};
	float m_breadth{};

	io::Logger m_logger{"World"};
};

} // namespace world

} // namespace fornani
