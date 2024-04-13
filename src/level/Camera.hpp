
#pragma once

#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include "../particle/Gravitator.hpp"

namespace automa {
struct ServiceProvider;
}

namespace fornani {

float const CAM_FRICTION = 0.85f;
float const CAM_MASS = 1.0f;
float const CAM_GRAV = 0.09f;

int const CX_OFFSET = 60;
int const CY_OFFSET = 60;

int const border_buffer{32};

class Camera {
  public:
	Camera();
	void update(automa::ServiceProvider& svc);
	void restrict_movement(sf::Vector2<float>& bounds);
	void fix_horizontally(sf::Vector2<float> map_dim);
	void fix_vertically(sf::Vector2<float> map_dim);
	void set_position(sf::Vector2<float> new_pos) { bounding_box.setPosition(new_pos); }
	void center(automa::ServiceProvider& svc, sf::Vector2<float> new_position);

	[[nodiscard]] auto get_observed_velocity() const -> sf::Vector2<float> { return observed_velocity; }
	[[nodiscard]] auto get_position() const -> sf::Vector2<float> { return bounding_box.getPosition(); }
	[[nodiscard]] auto within_frame(int x, int y) const -> bool { return (x > 0) && (x < screen_dimensions.x + border_buffer) && (y > 0) && (y < screen_dimensions.y + border_buffer); }

	private:
	vfx::Gravitator gravitator{};
	sf::Vector2<int> screen_dimensions{};
	sf::RectangleShape bounding_box{};
	sf::Vector2<float> observed_velocity{};
	sf::Vector2<float> previous_position{};

	float grav_force{CAM_GRAV};
};

} // namespace cam