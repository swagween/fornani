
#pragma once

#include <SFML/Graphics.hpp>
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

enum class LaserType : std::uint8_t { constant, repeater, oneoff };
enum class LaserAttributes : std::uint8_t { transcendent };

class Laser {
  public:
	Laser(automa::ServiceProvider& svc, sf::Vector2f position, LaserType type, int active = 128, int cooldown = 128, float size = 1.f);
	void update(automa::ServiceProvider& svc, player::Player& player, Map& map);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);
	void fire() { m_cooldown.start(); }

	[[nodiscard]] auto is(LaserType const test) const -> bool { return m_type == test; }

  private:
	sf::Vector2f calculate_size(Map& map);

	LaserType m_type{};
	HV m_direction{};
	shape::Shape m_hitbox;
	sf::RectangleShape m_drawbox{};
	util::Cooldown m_active;
	util::Cooldown m_cooldown;
	float m_size{};
};

} // namespace world

} // namespace fornani
