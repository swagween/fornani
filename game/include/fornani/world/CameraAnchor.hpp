
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/core/Fwd.hpp>

namespace fornani {

class CameraAnchor {
  public:
	CameraAnchor(sf::Vector2f position);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player);

  private:
	sf::Vector2f m_point{};
};

} // namespace fornani
