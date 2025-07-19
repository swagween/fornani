
#pragma once

#include <SFML/Graphics.hpp>
#include "fornani/components/PhysicsComponent.hpp"
#include "fornani/components/SteeringBehavior.hpp"
#include "fornani/graphics/CameraController.hpp"
#include "fornani/utils/BitFlags.hpp"
#include "fornani/utils/Cooldown.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani {

struct CameraShake {
	util::Cooldown timer{};
	util::Cooldown dampen{};
	graphics::ShakeProperties properties{};
};

enum class CameraFlags : std::uint8_t { shake };

class Camera {
  public:
	Camera();
	void update(automa::ServiceProvider& svc);
	void set_bounds(sf::Vector2f to_bounds);
	void center(sf::Vector2f new_position, float const force_multiplier = 1.f);
	void force_center(sf::Vector2f new_position);
	void begin_shake();

	[[nodiscard]] auto get_observed_velocity() const -> sf::Vector2f { return m_physics.apparent_velocity(); }
	[[nodiscard]] auto get_position() const -> sf::Vector2f { return m_final_position; }
	[[nodiscard]] auto within_frame(sf::Vector2f point) const -> bool { return m_bounds.contains(point); }

  private:
	[[nodiscard]] auto get_clamped_position(sf::Vector2f const position) const -> sf::Vector2f;
	components::PhysicsComponent m_physics{};
	components::SteeringBehavior m_steering{};
	float m_steering_force{};
	sf::FloatRect m_bounds{}; // map bounds
	sf::FloatRect m_view{};	  // window view (screen dimensions)

	sf::Vector2f m_final_position{};
	sf::Vector2f m_forced_target_position{}; // forced target, for use when camera is shaking or force-focused
	sf::Vector2f m_target_position{};		 // standard target, experiences bounds restrictions

	util::BitFlags<CameraFlags> m_flags{};
	graphics::CameraState m_state{};
	CameraShake m_shake{};
};

} // namespace fornani
