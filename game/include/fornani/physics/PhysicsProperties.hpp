
#pragma once

#include <SFML/Graphics.hpp>
#include <djson/json.hpp>

namespace fornani::shape {

struct PhysicsProperties {
	void unserialize(dj::Json const& in) {
		friction = sf::Vector2f{in["friction"][0].as<float>(), in["friction"][1].as<float>()};
		gravity = in["gravity"].as<float>();
		water_gravity_multiplier = in["water_gravity_multiplier"].as<float>();
		water_friction_multiplier = in["water_friction_multiplier"].as<float>();
		water_surface_multiplier = in["water_surface_multiplier"].as<float>();
	}
	sf::Vector2f friction{0.938f, 0.929f};
	float gravity{1.f};
	float water_gravity_multiplier{-0.5f};
	float water_friction_multiplier{0.9f};
	float water_surface_multiplier{0.995f};
	float water_sinking_multiplier{0.995f};
};

} // namespace fornani::shape
