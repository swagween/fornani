
#pragma once

#include <SFML/System/Vector2.hpp>
#include <fornani/utils/Cooldown.hpp>

namespace fornani {

struct ShakeParameters {
	float energy{};
	int frequency{};
	int lifetime{};
};

class Shaker {
  public:
	Shaker(ShakeParameters params);
	void tick(sf::Vector2f& point);
	void tick_static(sf::Vector2f& point);
	void set_point(sf::Vector2f point) { m_point = point; }
	void shake();

  private:
	ShakeParameters m_params;
	float m_energy;
	util::Cooldown m_phase;
	util::Cooldown m_lifetime;
	sf::Vector2f m_point{};
	sf::Vector2f m_offset{};
};

} // namespace fornani
