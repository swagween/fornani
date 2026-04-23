
#pragma once

#include <fornani/components/SteeringBehavior.hpp>
#include <fornani/entities/Mobile.hpp>
#include <fornani/io/Logger.hpp>

namespace fornani {

class TestMobile final : public Mobile {
  public:
	TestMobile(automa::ServiceProvider& svc, world::Map& map);
	void update(world::Map& map);
	void render(sf::RenderWindow& win, sf::Vector2f cam);
	void set_target(sf::Vector2f to) { m_target = to; }

	void debug();

  public:
	float max_vel{};
	float inertial_force{};
	float friction{};
	ThrustParameters thrust_params{};

  private:
	components::SteeringBehavior m_steering{};
	sf::Vector2f m_target{};

	io::Logger m_logger{"Test"};
};

} // namespace fornani
