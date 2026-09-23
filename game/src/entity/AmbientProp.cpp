
#include <fornani/entities/player/Player.hpp>
#include <fornani/entity/AmbientProp.hpp>
#include <fornani/graphics/Color.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/shader/FlatShader.hpp>

namespace fornani {

AmbientProp::AmbientProp(automa::ServiceProvider& svc, dj::Json const& in) : Entity{svc, in, "ambient_props"} {
	unserialize(in);
	auto const& in_data = svc.data.props[m_tag];
	m_params.emplace(svc, in_data);
	if (in["foreground"].as_bool()) { m_params->attributes.set(AmbientPropAttributes::foreground); }
	for (auto const& h : in_data["color_shift"]["hues"].as_array()) { m_params->hues.push_back(Color(h)); }
	m_params->shift_range = sf::Vector2f{in_data["color_shift"]["range"][0].as<float>(), in_data["color_shift"]["range"][1].as<float>()};
	m_sensor = components::CircleSensor{m_params->radius};
	p_animatable.set_texture(svc.assets.get_texture("ambient_prop_" + m_tag));
	p_animatable.set_dimensions(m_params->dimensions);
	p_animatable.tick();
	p_animatable.center();
	m_bob.physics.set_friction_componentwise({0.99f, 0.99f});
	m_sensor.set_position(get_global_center());
	if (m_params) {
		if (m_params->emitter) {
			m_emitter_cooldown.set_and_start(m_params->emitter->frequency);
			m_emitter_cooldown.randomize();
		}
		p_animatable.push_and_set_animation("basic", {0, m_params->num_frames, in_data["framerate"].as<int>(), -1});
	}
	m_textured = false;
	if (std::abs(m_variables.depth) < constants::tiny_value) { m_flags.set(AmbientPropFlags::interactable); }
	if (in_data["color_shift"]) { m_flags.set(AmbientPropFlags::flat_shaded); }
}

AmbientProp::AmbientProp(automa::ServiceProvider& svc, int channel, std::string_view tag) : Entity{svc, "ambient_props", 0}, m_tag{tag.data()}, m_channel{channel} { m_textured = false; }

std::unique_ptr<Entity> AmbientProp::clone() const { return std::make_unique<AmbientProp>(*this); }

void AmbientProp::serialize(dj::Json& out) {
	Entity::serialize(out);
	out["tag"] = m_tag;
	out["channel"] = m_channel;
	out["foreground"] = is_foreground();
	out["depth"] = m_variables.depth;
}

void AmbientProp::unserialize(dj::Json const& in) {
	Entity::unserialize(in);
	m_tag = in["tag"].as_string();
	m_channel = in["channel"].as<int>();
	m_variables.depth = in["depth"].as<float>();
}

void AmbientProp::expose() {
	Entity::expose();
	ImGui::SliderFloat("Depth", &m_variables.depth, -1.0, 1.0, "%.1f");
}

void AmbientProp::update(automa::ServiceProvider& svc, world::Map& map, SceneContext& context, player::Player& player) {
	if (spawn_denied()) { return; }
	Entity::update(svc, map, context, player);
	m_emitter_cooldown.update();
	if (m_params) {
		if (m_sensor.within_bounds(player.hurtbox) && m_flags.test(AmbientPropFlags::interactable)) {
			auto pvel = player.get_collider().physics.actual_velocity() * 0.0008f;
			m_bob.physics.apply_force(pvel);
		}
		m_bob.steering.seek(m_bob.physics, {}, m_params->sensitivity);
		m_bob.physics.simple_update();

		if (m_params->sensitivity > constants::tiny_value) {
			auto displacement = m_bob.physics.position.x;
			float normalized = std::tanh(displacement);
			auto frame = util::map_to_frame(normalized, -1.0f, 1.0f, 0, m_params->num_frames - 1);
			p_animatable.set_frame(frame);
		} else {
			p_animatable.tick();
		}
		p_animatable.set_channel(m_channel);
		if (m_params->emitter) {
			if (m_emitter_cooldown.is_almost_complete()) {
				map.spawn_emitter(svc, m_params->emitter->tag, get_global_center() + m_params->emitter->offset, {UND::up});
				m_emitter_cooldown.start(m_params->emitter->frequency);
			}
		}
	}
}

void AmbientProp::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	highlighted ? drawbox.setFillColor(sf::Color{60, 255, 120, 180}) : drawbox.setFillColor(sf::Color{60, 255, 120, 80});
	Entity::render(win, cam, size);
	if (!m_editor) {
		drawbox.setSize(get_world_dimensions() * size);
		drawbox.setPosition(get_world_dimensions() * size - cam);
		// win.draw(drawbox);
	}
	if (m_editor) {
		if (m_params) {
			p_animatable.set_scale(constants::f_scale_vec * size / constants::f_cell_size);
			p_animatable.set_position((get_f_grid_position() + m_params->offset / constants::f_cell_size) * size + cam + constants::f_cell_vec * 0.5f);
			p_animatable.set_frame(0);
			win.draw(p_animatable);
		}
		return;
	}
	if (spawn_denied() || is_flat_shaded()) { return; }
	if (m_params) {
		auto position = get_global_center() - cam + m_params->offset;

		auto const screen_center = win.getView().getSize() / 2.f;
		auto const displacement = position - screen_center;

		position += displacement * m_variables.depth * 0.1f;

		p_animatable.set_position(position);
		win.draw(p_animatable);
	}
	if (debug::is_debug()) {
		sf::CircleShape bob{};
		bob.setFillColor(sf::Color::Red);
		bob.setRadius(2.f);
		bob.setPosition(get_global_center() - cam + m_bob.physics.position);
		win.draw(bob);
		m_sensor.render(win, cam);
	}
}

void AmbientProp::flat_shade(sf::RenderWindow& win, sf::Vector2f cam, FlatShader& shader) {
	if (spawn_denied()) { return; }
	if (!is_flat_shaded()) {
		render(win, cam, 1.f);
		return;
	}
	if (m_params) {
		auto const alpha = std::clamp((m_variables.depth - m_params->shift_range.x) / (m_params->shift_range.y - m_params->shift_range.x), 0.f, 1.f);
		auto const color = gradient_color(m_params->hues, alpha);
		shader.finalize(color);
		auto position = get_global_center() - cam + m_params->offset;

		auto const screen_center = win.getView().getSize() / 2.f;
		auto const displacement = position - screen_center;

		position += displacement * m_variables.depth * 0.05f;

		p_animatable.set_position(position);
		shader.submit(win, p_animatable.get_sprite());
	}
}

AmbientPropParameters::AmbientPropParameters(automa::ServiceProvider& svc, dj::Json const& in) {
	num_frames = in["num_frames"].as<int>();
	sensitivity = in["sensitivity"].as<float>();
	radius = in["radius"].as<float>();
	dimensions = sf::Vector2i{in["dimensions"][0].as<int>(), in["dimensions"][1].as<int>()};
	if (in["destroy_effect"]) { destroy_effect.emplace(in["destroy_effect"].as_string()); }
	if (in["sound_effect"]) { destroy_effect.emplace(in["sound_effect"].as_string()); }
	in["foreground"].as_bool() ? attributes.set(AmbientPropAttributes::foreground) : attributes.reset(AmbientPropAttributes::foreground);
	in["destructible"].as_bool() ? attributes.set(AmbientPropAttributes::destructible) : attributes.reset(AmbientPropAttributes::destructible);
	in["audio"].as_bool() ? attributes.set(AmbientPropAttributes::audio) : attributes.reset(AmbientPropAttributes::audio);
	offset = sf::Vector2f{in["offset"][0].as<float>(), in["offset"][1].as<float>()};
	if (in["emitter"]) {
		auto params = vfx::EmitterParameters{};
		params.tag = in["emitter"]["tag"].as_string();
		params.frequency = in["emitter"]["frequency"].as<int>();
		emitter.emplace(params);
	}
}
} // namespace fornani
