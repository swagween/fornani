
#pragma once

#include <fornani/components/SteeringBehavior.hpp>
#include <fornani/entities/Mobile.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/utils/Flaggable.hpp>

namespace fornani {

enum class MobilePropFlags { dropped };

class MobileProp final : public Mobile, public Flaggable<MobilePropFlags> {
  public:
	MobileProp(automa::ServiceProvider& svc, world::Map& map, std::string_view tag, sf::Vector2i dimensions);
	void update(world::Map& map);
	void render(sf::RenderWindow& win, sf::Vector2f cam);
	void set_target(sf::Vector2f to) { m_target = to; }
	void push_and_set_animation(std::string_view tag, anim::Parameters params) { p_animatable.push_and_set_animation(tag, params); }
	void push_animation(std::string_view tag, anim::Parameters params) { p_animatable.push_animation(tag, params); }
	void set_animation(std::string_view to) { p_animatable.set_animation(to); }
	void drop() { set_flag(MobilePropFlags::dropped); };

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
