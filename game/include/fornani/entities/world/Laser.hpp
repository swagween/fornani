
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/utils/Circuit.hpp>
#include <fornani/utils/Cooldown.hpp>
#include <fornani/utils/Direction.hpp>
#include <fornani/utils/Shape.hpp>

namespace fornani {

namespace automa {
struct ServiceProvider;
}

namespace player {
class Player;
}

namespace world {

class Map;

enum class LaserType : std::uint8_t { turret, magic };
enum class LaserAttributes : std::uint8_t { transcendent };

class Laser {
  public:
	Laser(automa::ServiceProvider& svc, Map& map, sf::Vector2f position, LaserType type, HV direction, int active = 128, int cooldown = 128, float size = 1.f);
	void update(automa::ServiceProvider& svc, player::Player& player, Map& map);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);
	void fire() { m_active.start(); }

	[[nodiscard]] auto is(LaserType const test) const -> bool { return m_type == test; }
	[[nodiscard]] auto is_active() const -> bool { return m_active.running(); }
	[[nodiscard]] auto is_complete() const -> bool { return m_cooldown.is_almost_complete(); }

  private:
	sf::Vector2f calculate_size(Map& map);

	LaserType m_type{};
	HV m_direction{};
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
