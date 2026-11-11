
#pragma once

#include <fornani/components/SteeringComponent.hpp>
#include <fornani/core/Fwd.hpp>
#include <fornani/utils/Cooldown.hpp>

namespace fornani {

class DogfightController {
  public:
	void update(automa::ServiceProvider& svc);
	void handle_input(input::InputSystem& input);
	void set_position(sf::Vector2f const to) { m_target = to; }
	void spiral() { m_target.y += 0.8f; }

	[[nodiscard]] auto get_position() const -> sf::Vector2f { return m_steering.physics.position; }
	[[nodiscard]] auto get_target() const -> sf::Vector2f { return m_target; }
	[[nodiscard]] auto get_physics() const -> components::PhysicsComponent const& { return m_steering.physics; }
	[[nodiscard]] auto is_dashing() const -> bool { return m_dash.get_normalized() > 0.5f; }

  private:
	components::SteeringComponent m_steering{};
	sf::Vector2f m_target{};
	sf::Vector2f m_speed{1.0f, 1.2f};

	util::Cooldown m_dash{200};
};

} // namespace fornani
