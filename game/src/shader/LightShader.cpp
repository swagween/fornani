
#include <fornani/setup/ResourceFinder.hpp>
#include <fornani/shader/LightShader.hpp>

#include <imgui.h>

// its important this exactly matches the value in shaders/light_combined.frag
#define MAX_POINT_LIGHTS 5

namespace fornani {

void PointLight::update() {
	if (m_has_steering) {
		m_steering.seek(m_physics, world_position, m_steering_force);
		m_steering.smooth_random_walk(m_physics, m_steering_dampen, m_steering_radius);
		m_physics.simple_update();
	} else {
		m_physics.position = world_position;
	}
	m_counter.update();
	auto adjustment = std::sin(m_counter.get() * flicker_rate);
	attenuation_linear += adjustment * flicker_radius;
}

void PointLight::unserialize(dj::Json const& in) {
	luminosity = in["luminosity"].as<float>();
	radius = in["radius"].as<float>();
	attenuation_constant = in["attenuation_constant"].as<float>();
	attenuation_linear = in["attenuation_linear"].as<float>();
	attenuation_quadratic = in["attenuation_quadratic"].as<float>();
	distance_scaling = in["distance_scaling"].as<float>();
	distance_flat = in["distance_flat"].as<float>();
	flicker_rate = in["flicker_rate"].as<float>();
	flicker_radius = in["flicker_radius"].as<float>();
	if (in["steering"].is_object()) {
		m_has_steering = true;
		m_steering_force = in["steering"]["force"].as<float>();
		m_steering_dampen = in["steering"]["dampen"].as<float>();
		m_steering_radius = in["steering"]["radius"].as<float>();
		m_physics.set_global_friction(in["steering"]["friction"].as<float>());
	}
}

LightShader::LightShader(ResourceFinder& finder) {
	if (!sf::Shader::isAvailable()) { NANI_LOG_WARN(m_logger, "Shaders are not available."); }
	auto vert = finder.paths.resources / fs::path{"shader/shaders/point_light.vert"};
	auto frag = finder.paths.resources / fs::path{"shader/shaders/light_combined.frag"};
	if (!m_shader.loadFromFile(vert, frag)) { NANI_LOG_WARN(m_logger, "Failed to load shader {}", frag.string()); }
}
void LightShader::AddPointLight(sf::Vector2f position, int luminosity, float radius, float att_c, float att_l, float att_q, float distanceScaling, float distanceFlat) {
	if (currentPointLight >= (MAX_POINT_LIGHTS - 1)) {
		NANI_LOG_WARN(m_logger, "Maximum lights reached.");
		return;
	}

	pointlightPosition.push_back(position);
	pointlightLuminosity.push_back(luminosity);
	pointlightRadius.push_back(radius);
	pointlightAttenuation_constant.push_back(att_c);
	pointlightAttenuation_linear.push_back(att_l);
	pointlightAttenuation_quadratic.push_back(att_q);
	pointlightDistanceScaling.push_back(distanceScaling);
	pointlightDistanceFlat.push_back(distanceFlat);

	currentPointLight++;
}
void LightShader::AddSpotLight(sf::Vector2f position, sf::Vector2f direction, int luminosity, float radius, float att_c, float att_l, float att_q, float cutoff, float outerCutoff, float distanceScaling, float distanceFlat) {

	if (currentSpotLight >= (MAX_POINT_LIGHTS - 1)) {
		NANI_LOG_WARN(m_logger, "Maximum lights reached.");
		return;
	}
	spotlightPosition.push_back(position);
	spotlightDirection.push_back(direction);
	spotlightLuminosity.push_back(luminosity);
	spotlightRadius.push_back(radius);
	spotlightAttenuation_constant.push_back(att_c);
	spotlightAttenuation_linear.push_back(att_l);
	spotlightAttenuation_quadratic.push_back(att_q);
	spotlight_cutoff.push_back(cutoff);
	spotlight_outerCutoff.push_back(outerCutoff);
	spotlightDistanceScaling.push_back(distanceScaling);
	spotlightDistanceFlat.push_back(distanceFlat);

	currentSpotLight++;
}

void LightShader::Finalize() {
	m_shader.setUniform("u_px", m_scale);
	m_shader.setUniform("u_tex_size", sf::Glsl::Vec2{m_texture_size});
	m_shader.setUniform("u_parity", sf::Glsl::Vec2{m_parity});

	m_shader.setUniform("pointlight_count", currentPointLight + 1);
	m_shader.setUniformArray("pointlight_position", pointlightPosition.data(), pointlightPosition.size());
	m_shader.setUniformArray("pointlight_luminence", pointlightLuminosity.data(), pointlightLuminosity.size());
	m_shader.setUniformArray("pointlight_radius", pointlightRadius.data(), pointlightRadius.size());
	m_shader.setUniformArray("pointlight_attenuation_constant", pointlightAttenuation_constant.data(), pointlightAttenuation_constant.size());
	m_shader.setUniformArray("pointlight_attenuation_linear", pointlightAttenuation_linear.data(), pointlightAttenuation_linear.size());
	m_shader.setUniformArray("pointlight_attenuation_quadratic", pointlightAttenuation_quadratic.data(), pointlightAttenuation_quadratic.size());
	m_shader.setUniformArray("pointlight_distance_scaling", pointlightDistanceScaling.data(), pointlightDistanceScaling.size());
	m_shader.setUniformArray("pointlight_distance_flat", pointlightDistanceFlat.data(), pointlightDistanceFlat.size());

	m_shader.setUniform("spotlight_count", currentSpotLight + 1);
	m_shader.setUniformArray("spotlight_position", spotlightPosition.data(), spotlightPosition.size());
	m_shader.setUniformArray("spotlight_direction", spotlightDirection.data(), spotlightDirection.size());
	m_shader.setUniformArray("spotlight_luminence", spotlightLuminosity.data(), spotlightLuminosity.size());
	m_shader.setUniformArray("spotlight_radius", spotlightRadius.data(), spotlightRadius.size());
	m_shader.setUniformArray("spotlight_attenuation_constant", spotlightAttenuation_constant.data(), spotlightAttenuation_constant.size());
	m_shader.setUniformArray("spotlight_attenuation_linear", spotlightAttenuation_linear.data(), spotlightAttenuation_linear.size());
	m_shader.setUniformArray("spotlight_attenuation_quadratic", spotlightAttenuation_quadratic.data(), spotlightAttenuation_quadratic.size());
	m_shader.setUniformArray("spotlight_cutoff", spotlight_cutoff.data(), spotlight_cutoff.size());
	m_shader.setUniformArray("spotlight_outerCutoff", spotlight_outerCutoff.data(), spotlight_outerCutoff.size());
	m_shader.setUniformArray("spotlight_distance_scaling", spotlightDistanceScaling.data(), spotlightDistanceScaling.size());
	m_shader.setUniformArray("spotlight_distance_flat", spotlightDistanceFlat.data(), spotlightDistanceFlat.size());

	m_shader.setUniform("u_darken", m_darken_factor);
}

void LightShader::Submit(sf::RenderWindow& win, Palette& palette, sf::Sprite const& sprite) {
	m_shader.setUniform("palette_size", static_cast<int>(palette.get_size()));
	m_shader.setUniform("palette", palette.get_texture().getTexture());
	m_shader.setUniform("texture", sprite.getTexture());

	win.draw(sprite, &m_shader);
}

void LightShader::ClearPointLights() {
	currentPointLight = 0;
	pointlightPosition.clear();
	pointlightLuminosity.clear();
	pointlightRadius.clear();
	pointlightAttenuation_constant.clear();
	pointlightAttenuation_linear.clear();
	pointlightAttenuation_quadratic.clear();
	pointlightDistanceScaling.clear();
}

void LightShader::ClearSpotLights() {
	currentSpotLight = 0;
	spotlightPosition.clear();
	spotlightDirection.clear();
	spotlightLuminosity.clear();
	spotlightRadius.clear();
	spotlightAttenuation_constant.clear();
	spotlightAttenuation_linear.clear();
	spotlightAttenuation_quadratic.clear();
	spotlight_cutoff.clear();
	spotlight_outerCutoff.clear();
	spotlightDistanceScaling.clear();
}

} // namespace fornani
