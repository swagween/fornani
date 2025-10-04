#pragma once

#include <SFML/Graphics.hpp>
#include <djson/json.hpp>
#include <fornani/components/PhysicsComponent.hpp>
#include <fornani/components/SteeringBehavior.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/shader/Palette.hpp>
#include <fornani/utils/Constants.hpp>
#include <fornani/utils/Counter.hpp>
#include <fornani/utils/Random.hpp>

#define USING_LIGHT_COLORS false

namespace fornani {

struct PointLight {
	PointLight() = default;
	PointLight(dj::Json const& in, sf::Vector2f pos) : world_position{pos} {
		unserialize(in);
		m_physics.velocity = random::random_vector_float(-1.f, 1.f);
		m_physics.position = pos;
	}
	void update();
	[[nodiscard]] auto get_position() const -> auto { return m_physics.position; }
	sf::Vector2f position;
	sf::Vector2f world_position;

#if USING_LIGHT_COLORS
	sf::Color color; // not really set up, but its possible to shift/mix/blend/whatever colors while maintaining the palette shift
#endif

	float luminosity;
	float radius;
	float attenuation_constant;
	float attenuation_linear;
	float attenuation_quadratic;
	float distance_scaling;
	float distance_flat;
	float flicker_rate;
	float flicker_radius;
	void unserialize(dj::Json const& in);

  private:
	components::SteeringBehavior m_steering{};
	components::PhysicsComponent m_physics{};
	util::FloatCounter m_counter{};
	float m_steering_dampen{};
	float m_steering_radius{};
	float m_steering_force{};
	bool m_has_steering{};
};
struct SpotLight {
	sf::Vector2f position;
	sf::Vector2f direction;
	float radius;
	float luminosity;
	float attenuation_constant;
	float attenuation_linear;
	float attenuation_quadratic;
	float cutoffAngle;
	float outerCutoffAngle;
	float distanceScaling;
	float distanceFlat;
};

class ResourceFinder;
class LightShader {
  public:
	LightShader(ResourceFinder& finder);

	void ClearPointLights();
	void ClearSpotLights();

	void AddPointLight(sf::Vector2f position, int luminosity, float radius, float att_c, float att_l, float att_q, float distanceScaling, float distanceFlat);
	void AddPointLight(PointLight pointlight) {
		AddPointLight(pointlight.position, pointlight.luminosity, pointlight.radius, pointlight.attenuation_constant, pointlight.attenuation_linear, pointlight.attenuation_quadratic, pointlight.distance_scaling, pointlight.distance_flat);
	}
	void AddSpotLight(sf::Vector2f position, sf::Vector2f direction, int luminosity, float radius, float att_c, float att_l, float att_q, float cutoff, float outerCutoff, float distanceScaling, float distanceFlat);
	void AddSpotLight(SpotLight spotlight) {
		AddSpotLight(spotlight.position, spotlight.direction, spotlight.luminosity, spotlight.radius, spotlight.attenuation_constant, spotlight.attenuation_linear, spotlight.attenuation_quadratic, spotlight.cutoffAngle,
					 spotlight.outerCutoffAngle, spotlight.distanceScaling, spotlight.distanceFlat);
	}

	void set_scale(float const to) { m_scale = to; }
	void set_darken(float const to) { m_darken_factor = to; }
	void set_texture_size(sf::Vector2f const to) { m_texture_size = to; }
	void set_parity(sf::Vector2i const reference) { m_parity = sf::Vector2f{static_cast<float>(std::abs(reference.x) % 2), static_cast<float>(std::abs(reference.y) % 2)}; }

	void Finalize();
	void Submit(sf::RenderWindow& win, Palette& palette, sf::Sprite const& sprite);

	void debug();

	// if youd like some additional safety, add a boolean, create a BeginShader function, set the boolean to true, and turn it off at the end of submit
	// you could potentially use a wrapper object as well, that calls Begin, AddLight or whatever on construction, then finalize and submit on deconstruction

	int currentPointLight = 0;
	int currentSpotLight = 0;

	std::vector<sf::Vector2f> pointlightPosition{};
	std::vector<float> pointlightLuminosity{};
	std::vector<float> pointlightRadius{};
	std::vector<float> pointlightAttenuation_constant{};
	std::vector<float> pointlightAttenuation_linear{};
	std::vector<float> pointlightAttenuation_quadratic{};
	std::vector<float> pointlightDistanceScaling{};
	std::vector<float> pointlightDistanceFlat{};

	std::vector<sf::Vector2f> spotlightPosition{};
	std::vector<sf::Vector2f> spotlightDirection{};
	std::vector<float> spotlightLuminosity{};
	std::vector<float> spotlightRadius{};
	std::vector<float> spotlightAttenuation_constant{};
	std::vector<float> spotlightAttenuation_linear{};
	std::vector<float> spotlightAttenuation_quadratic{};
	std::vector<float> spotlight_cutoff{};
	std::vector<float> spotlight_outerCutoff{};
	std::vector<float> spotlightDistanceScaling{};
	std::vector<float> spotlightDistanceFlat{};

  private:
	sf::Shader m_shader{};

	float m_scale{2.f};
	float m_darken_factor{2.f};
	sf::Vector2f m_texture_size{};
	sf::Vector2f m_parity{};

	io::Logger m_logger{"shader"};
};

} // namespace fornani
