
#pragma once

#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>
#include "../components/PhysicsComponent.hpp"
#include "../utils/Random.hpp"

namespace cam {

float const CAM_FRICTION = 0.75f;
float const CAM_MASS = 1.0f;
float const CAM_GRAV = 0.003f;

int const CX_OFFSET = 60;
int const CY_OFFSET = 60;

int const SHAKE_FACTOR = 8;
int const SHAKE_VOLATILITY = 12;
int const SHAKE_DURATION = 100;

int const border_buffer{32};

float const TINY_VALUE = 0.000001f;

inline const sf::Vector2<uint32_t> aspect_ratio{3840, 2160};
inline const sf::Vector2<uint32_t> screen_dimensions{aspect_ratio.x / 4, aspect_ratio.y / 4};

class Camera {
  public:
	Camera() {
		physics = components::PhysicsComponent({CAM_FRICTION, CAM_FRICTION}, CAM_MASS);
		bounding_box = sf::Rect<float>({0.0f, 0.0f}, {(float)screen_dimensions.x, (float)screen_dimensions.y});
	}

	void move(sf::Vector2<float> new_position) {
		physics.apply_force(physics.position - new_position);
		update();
	}

	void update() {

		physics.update_dampen();
		bounding_box.left = physics.position.x;
		bounding_box.top = physics.position.y;
		if (bounding_box.top < 0.0f) {
			bounding_box.top = 0.0f;
			physics.position.y = 0.0f;
		}
		if (bounding_box.left < 0.0f) {
			bounding_box.left = 0.0f;
			physics.position.x = 0.0f;
		}
		if (shaking) {
			if (shake_counter % SHAKE_VOLATILITY == 0) { shake(); }
			shake_counter++;
			if (shake_counter > SHAKE_DURATION) {
				shake_counter = 0;
				shaking = false;
			}
		}
	}

	void restrict_movement(sf::Vector2<float>& bounds) {
		if (bounding_box.top + bounding_box.height > bounds.y) {
			bounding_box.top = bounds.y - bounding_box.height;
			physics.position.y = bounds.y - bounding_box.height;
		}
		if (bounding_box.left + bounding_box.width > bounds.x) {
			bounding_box.left = bounds.x - bounding_box.width;
			physics.position.x = bounds.x - bounding_box.width;
		}
	}

	void fix_horizontally(sf::Vector2<float> map_dim) {
		bounding_box.top = (map_dim.y - screen_dimensions.y) / 2;
		physics.position.y = bounding_box.top;
	}

	void fix_vertically(sf::Vector2<float> map_dim) {
		bounding_box.left = (map_dim.x - screen_dimensions.x) / 2;
		physics.position.x = bounding_box.left;
	}

	void set_position(sf::Vector2<float> new_pos) { physics.position = new_pos; }

	void center(sf::Vector2<float> new_position) {
		float gx = physics.position.x;
		float gy = physics.position.y;
		float mx = new_position.x - bounding_box.width / 2;
		float my = new_position.y - bounding_box.height / 2;

		float force_x = mx - gx;
		float force_y = my - gy;
		float mag = sqrt((force_x * force_x) + (force_y * force_y));
		float str = CAM_GRAV / mag * mag;
		force_x *= str;
		force_y *= str;
		physics.apply_force({force_x, force_y});
		update();
	}

	bool within_frame(int x, int y) { return (x > 0) && (x < screen_dimensions.x + border_buffer) && (y > 0) && (y < screen_dimensions.y + border_buffer); }

	void begin_shake() { shaking = true; }

	void shake() {
		util::Random rand{};
		float nudge_x = rand.random_range(-SHAKE_FACTOR, SHAKE_FACTOR);
		float nudge_y = rand.random_range(-SHAKE_FACTOR, SHAKE_FACTOR);
		physics.velocity.x = nudge_x * 0.1;
		physics.velocity.y = nudge_y * 0.1;
	}

	sf::Rect<float> bounding_box{};
	components::PhysicsComponent physics{};
	sf::Vector2<float> observed_velocity{};
	sf::Vector2<float> position{};
	sf::Vector2<float> previous_position{};

	int shake_counter{};
	bool shaking = false;
};

} // namespace cam