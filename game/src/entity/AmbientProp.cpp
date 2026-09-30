
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
	init(svc);
}

AmbientProp::AmbientProp(automa::ServiceProvider& svc, int channel, std::string_view tag, float depth) : Entity{svc, "ambient_props", 0}, m_tag{tag.data()}, m_channel{channel}, m_variables{.depth{depth}} { init(svc); }

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
	ImGui::InputInt("Channel", &m_channel);
	ImGui::SliderFloat("Depth", &m_variables.depth, -1.0, 1.0, "%.1f");
}

void AmbientProp::update(automa::ServiceProvider& svc, world::Map& map, SceneContext& context, player::Player& player) {
	if (spawn_denied() || is_destroyed()) { return; }
	Entity::update(svc, map, context, player);
	if (map.has_property(world::MapProperties::lighting)) { m_flags.set(AmbientPropFlags::light_shaded); }
	m_emitter_cooldown.update();
	if (m_params) {
		if (m_sensor.within_bounds(player.hurtbox) && m_flags.test(AmbientPropFlags::interactable)) {
			auto pvel = player.get_collider().physics.actual_velocity() * 0.0008f;
			m_bob.physics.apply_force(pvel);
		}
		m_bob.steering.seek(m_bob.physics, {}, m_params->sensitivity);
		m_bob.physics.simple_update();

		if (m_params->animations.empty()) {
			if (m_params->sensitivity > constants::tiny_value) {
				auto displacement = m_bob.physics.position.x;
				float normalized = std::tanh(displacement);
				auto frame = util::map_to_frame(normalized, -1.0f, 1.0f, 0, m_params->num_frames - 1);
				p_animatable.set_frame(frame);
			} else {
				p_animatable.tick();
			}
		} else {
			p_animatable.tick();
			if (p_animatable.is_complete()) {
				auto const& animations = m_params->animations;
				auto const& current = p_animatable.get_animation_tag();

				auto const it = std::ranges::find(animations, current);
				auto const next = std::next(it) == animations.end() ? animations.begin() : std::next(it);

				p_animatable.set_animation(*next);
			}
		}
		p_animatable.set_channel(m_channel);
		if (m_params->emitter) {
			if (m_emitter_cooldown.is_almost_complete()) {
				map.spawn_emitter(svc, m_params->emitter->tag, get_global_center() + m_params->emitter->offset, {UND::up});
				m_emitter_cooldown.start(m_params->emitter->frequency);
			}
		}
		if (is_destructible() && is_interactable()) {
			for (auto& p : map.active_projectiles) {
				if (m_sensor.within_bounds(p.get_collider())) {
					m_health->inflict(p.get_power());
					if (is_destroyed() && m_params->destroy_effect) { map.spawn_effect(svc, m_params->destroy_effect.value(), m_sensor.bounds.getPosition(), p.get_velocity() * 0.001f); }
				}
			}
		}

		// interpret color gradient based on time of day, if relevant
		auto const alpha = std::clamp((m_variables.depth - m_params->shift_range.x) / (m_params->shift_range.y - m_params->shift_range.x), 0.f, 1.f);
		if (!m_params->hues.empty()) {
			m_color = gradient_color<AmbientColor>(m_params->hues, alpha, [&map](AmbientColor const& c) { return c.tile_index != 0 ? map.get_actual_tile_color(c.tile_index) : static_cast<sf::Color>(c); });
		}
	}
}

void AmbientProp::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	highlighted ? drawbox.setFillColor(sf::Color{60, 255, 120, 180}) : drawbox.setFillColor(sf::Color{60, 255, 120, 80});
	Entity::render(win, cam, size);
	if (!m_editor && debug::is_debug()) {
		drawbox.setSize(get_world_dimensions() * size);
		drawbox.setPosition(get_world_dimensions() * size - cam);
		win.draw(drawbox);
	}
	if (m_editor) {
		if (m_params) {
			p_animatable.set_scale(constants::f_scale_vec * size / constants::f_cell_size);
			p_animatable.set_position((get_f_grid_position() + m_params->offset / constants::f_cell_size) * size + cam + constants::f_cell_vec * 0.5f);
			p_animatable.set_frame(0);
			p_animatable.set_channel(m_channel);
			win.draw(p_animatable);
		}
		return;
	}
	if (spawn_denied() || is_flat_shaded() || is_destroyed()) { return; }
	if (m_params) {
		p_animatable.set_position(generate_position(win, cam));
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

void AmbientProp::render(sf::RenderWindow& win, sf::RenderTexture& tex, sf::Vector2f cam) {
	if (spawn_denied() || is_destroyed()) { return; }
	p_animatable.set_scale(constants::f_scale_vec);
	p_animatable.set_position(generate_position(win, cam * 0.05f));
	tex.draw(p_animatable);
	++debug::draw_calls;
}

void AmbientProp::flat_shade(sf::RenderWindow& win, sf::Vector2f cam, FlatShader& shader) {
	if (spawn_denied() || is_destroyed()) { return; }
	if (!is_flat_shaded()) {
		render(win, cam, 1.f);
		return;
	}
	if (m_params) {
		shader.finalize(m_color);
		p_animatable.set_position(generate_position(win, cam));
		shader.submit(win, p_animatable.get_sprite());
	}
}

void AmbientProp::init(automa::ServiceProvider& svc) {
	auto const& in_data = svc.data.props[m_tag];
	m_params.emplace(svc, in_data);
	if (in_data["health"]) { m_health.emplace(in_data["health"].as<float>()); }
	for (auto const& h : in_data["color_shift"]["hues"].as_array()) { m_params->hues.push_back(AmbientColor{h["color"], h["tile"].as<int>()}); }
	for (auto const& a : in_data["animation"].as_array()) {
		m_params->animations.push_back(a["label"].as_string());
		p_animatable.push_and_set_animation(a["label"].as_string(), anim::Parameters::from_json(a["params"]));
	}

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
		if (m_params->animations.empty()) { p_animatable.push_and_set_animation("basic", {0, m_params->num_frames, in_data["framerate"].as<int>(), -1}); }
	}
	m_textured = false;
	if (std::abs(m_variables.depth) < constants::tiny_value) { m_flags.set(AmbientPropFlags::interactable); }
	if (in_data["color_shift"]) { m_flags.set(AmbientPropFlags::flat_shaded); }
}

auto AmbientProp::generate_position(sf::RenderWindow& win, sf::Vector2f cam) const -> sf::Vector2f {
	auto position = get_global_center() - cam + m_params->offset;

	auto const screen_center = win.getView().getSize() / 2.f;
	auto const displacement = position - screen_center;

	position += displacement * m_variables.depth * m_params->depth_multiplier;
	return position;
}

AmbientPropParameters::AmbientPropParameters(automa::ServiceProvider& svc, dj::Json const& in) {
	num_frames = in["num_frames"].as<int>();
	sensitivity = in["sensitivity"].as<float>();
	if (in["depth_multiplier"]) { depth_multiplier = in["depth_multiplier"].as<float>(); }
	radius = in["radius"].as<float>();
	dimensions = sf::Vector2i{in["dimensions"][0].as<int>(), in["dimensions"][1].as<int>()};
	if (in["destroy_effect"]) { destroy_effect.emplace(in["destroy_effect"].as_string()); }
	if (in["sound_effect"]) { destroy_effect.emplace(in["sound_effect"].as_string()); }
	in["foreground"].as_bool() ? attributes.set(AmbientPropAttributes::foreground) : attributes.reset(AmbientPropAttributes::foreground);
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
