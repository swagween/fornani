
#include <fornani/setup/ResourceFinder.hpp>
#include <fornani/shader/LightShader.hpp>

#include <imgui.h>

// its important this exactly matches the value in shaders/light_combined.frag
#define MAX_POINT_LIGHTS 5

namespace fornani {

constexpr auto max_light_v = 3.f;

auto linear_debug = 3.0f;
auto lumin_debug = 2.5f;
auto rad_debug = 0.39f;

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
	luminosity = lumin_debug;
	luminosity = in["luminosity"].as<float>();
	radius = rad_debug;
	radius = in["radius"].as<float>();
	attenuation_constant = in["attenuation_constant"].as<float>();
	attenuation_linear = linear_debug;
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

void LightShader::add_point_light(sf::Vector2f position, int luminosity, float radius, float att_c, float att_l, float att_q, float distance_scaling, float distance_flat) {
	if (current_point_light >= (MAX_POINT_LIGHTS - 1)) {
		NANI_LOG_WARN(m_logger, "Maximum lights reached.");
		return;
	}

	pointlight_position.push_back(position);
	pointlight_luminosity.push_back(luminosity);
	pointlight_radius.push_back(radius);
	pointlight_attenuation_constant.push_back(att_c);
	pointlight_attenuation_linear.push_back(att_l);
	pointlight_attenuation_quadratic.push_back(att_q);
	pointlight_distance_scaling.push_back(distance_scaling);
	pointlight_distance_flat.push_back(distance_flat);

	current_point_light++;
}

void LightShader::add_spotlight(sf::Vector2f position, sf::Vector2f direction, int luminosity, float radius, float att_c, float att_l, float att_q, float cutoff, float outer_cutoff, float distance_scaling, float distance_flat) {
	if (current_spotlight >= (MAX_POINT_LIGHTS - 1)) {
		NANI_LOG_WARN(m_logger, "Maximum lights reached.");
		return;
	}

	spotlight_position.push_back(position);
	spotlight_direction.push_back(direction);
	spotlight_luminosity.push_back(luminosity);
	spotlight_radius.push_back(radius);
	spotlight_attenuation_constant.push_back(att_c);
	spotlight_attenuation_linear.push_back(att_l);
	spotlight_attenuation_quadratic.push_back(att_q);
	spotlight_cutoff.push_back(cutoff);
	spotlight_outer_cutoff.push_back(outer_cutoff);
	spotlight_distance_scaling.push_back(distance_scaling);
	spotlight_distance_flat.push_back(distance_flat);

	current_spotlight++;
}

void LightShader::finalize() {
	m_shader.setUniform("u_px", m_scale);
	m_shader.setUniform("u_tex_size", sf::Glsl::Vec2{m_texture_size});
	m_shader.setUniform("u_parity", sf::Glsl::Vec2{m_parity});
	m_shader.setUniform("u_max_light", max_light_v);

	m_shader.setUniform("pointlight_count", current_point_light + 1);
	m_shader.setUniformArray("pointlight_position", pointlight_position.data(), pointlight_position.size());
	m_shader.setUniformArray("pointlight_luminence", pointlight_luminosity.data(), pointlight_luminosity.size());
	m_shader.setUniformArray("pointlight_radius", pointlight_radius.data(), pointlight_radius.size());
	m_shader.setUniformArray("pointlight_attenuation_constant", pointlight_attenuation_constant.data(), pointlight_attenuation_constant.size());
	m_shader.setUniformArray("pointlight_attenuation_linear", pointlight_attenuation_linear.data(), pointlight_attenuation_linear.size());
	m_shader.setUniformArray("pointlight_attenuation_quadratic", pointlight_attenuation_quadratic.data(), pointlight_attenuation_quadratic.size());
	m_shader.setUniformArray("pointlight_distance_scaling", pointlight_distance_scaling.data(), pointlight_distance_scaling.size());
	m_shader.setUniformArray("pointlight_distance_flat", pointlight_distance_flat.data(), pointlight_distance_flat.size());

	m_shader.setUniform("spotlight_count", current_spotlight + 1);
	m_shader.setUniformArray("spotlight_position", spotlight_position.data(), spotlight_position.size());
	m_shader.setUniformArray("spotlight_direction", spotlight_direction.data(), spotlight_direction.size());
	m_shader.setUniformArray("spotlight_luminence", spotlight_luminosity.data(), spotlight_luminosity.size());
	m_shader.setUniformArray("spotlight_radius", spotlight_radius.data(), spotlight_radius.size());
	m_shader.setUniformArray("spotlight_attenuation_constant", spotlight_attenuation_constant.data(), spotlight_attenuation_constant.size());
	m_shader.setUniformArray("spotlight_attenuation_linear", spotlight_attenuation_linear.data(), spotlight_attenuation_linear.size());
	m_shader.setUniformArray("spotlight_attenuation_quadratic", spotlight_attenuation_quadratic.data(), spotlight_attenuation_quadratic.size());
	m_shader.setUniformArray("spotlight_cutoff", spotlight_cutoff.data(), spotlight_cutoff.size());
	m_shader.setUniformArray("spotlight_outerCutoff", spotlight_outer_cutoff.data(), spotlight_outer_cutoff.size());
	m_shader.setUniformArray("spotlight_distance_scaling", spotlight_distance_scaling.data(), spotlight_distance_scaling.size());
	m_shader.setUniformArray("spotlight_distance_flat", spotlight_distance_flat.data(), spotlight_distance_flat.size());

	m_shader.setUniform("u_darken", m_darken_factor);
}

void LightShader::submit(sf::RenderWindow& win, Palette& palette, sf::Sprite const& sprite) {
	m_shader.setUniform("palette_size", static_cast<int>(palette.get_size()));
	m_shader.setUniform("palette", palette.get_texture().getTexture());
	m_shader.setUniform("texture", sprite.getTexture());

	win.draw(sprite, &m_shader);
}

void LightShader::clear_point_lights() {
	current_point_light = -1;
	pointlight_position = {};
	pointlight_luminosity.clear();
	pointlight_radius.clear();
	pointlight_attenuation_constant.clear();
	pointlight_attenuation_linear.clear();
	pointlight_attenuation_quadratic.clear();
	pointlight_distance_scaling.clear();
	pointlight_distance_flat.clear();
}

void LightShader::clear_spotlights() {
	current_spotlight = 0;
	spotlight_position.clear();
	spotlight_direction.clear();
	spotlight_luminosity.clear();
	spotlight_radius.clear();
	spotlight_attenuation_constant.clear();
	spotlight_attenuation_linear.clear();
	spotlight_attenuation_quadratic.clear();
	spotlight_cutoff.clear();
	spotlight_outer_cutoff.clear();
	spotlight_distance_scaling.clear();
}

void LightShader::debug() {
	ImGui::SliderFloat("Luminosity: ", &lumin_debug, 0.f, 400.f);
	ImGui::SliderFloat("Attenuation: ", &linear_debug, 0.f, 400.f);
	ImGui::SliderFloat("Redius: ", &rad_debug, 0.f, 100.f);
}

} // namespace fornani
