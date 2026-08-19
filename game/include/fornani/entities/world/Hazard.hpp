
#pragma once

#include <djson/json.hpp>
#include <fornani/components/CircleSensor.hpp>
#include <fornani/graphics/Transition.hpp>
#include <fornani/utils/Flaggable.hpp>

namespace fornani {

namespace world {
class Map;
}

namespace player {
class Player;
}

enum class HazardFlags : std::uint8_t { reset };

class Hazard : public Flaggable<HazardFlags> {
  public:
	Hazard(dj::Json const& in, sf::Vector2f position, sf::Vector2f center, sf::Angle rotation = {});
	Hazard(sf::Vector2f position, float radius);
	void update(player::Player& player, world::Map& map, graphics::Transition& transition);
	void render(sf::RenderWindow& win, sf::Vector2f cam);

  private:
	components::CircleSensor m_sensor{};
};

} // namespace fornani
