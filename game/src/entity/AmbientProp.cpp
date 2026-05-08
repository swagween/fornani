
#include <fornani/entities/player/Player.hpp>
#include <fornani/entity/AmbientProp.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani {

AmbientProp::AmbientProp(automa::ServiceProvider& svc, dj::Json const& in) : Entity{svc, in, "ambient_props"} {
	unserialize(in);
	m_params.emplace(svc.data.props[m_tag]);
	if (in["foreground"].as_bool()) { m_params->attributes.set(AmbientPropAttributes::foreground); }
	m_sensor = components::CircleSensor{m_params->radius};
	Animatable::set_texture(svc.assets.get_texture("ambient_prop_" + m_tag));
	Animatable::set_dimensions(m_params->dimensions);
	tick();
	Animatable::center();
	Animatable::push_and_set_animation("basic", {0, 9, 1, -1});
	m_bob.physics.set_friction_componentwise({0.99f, 0.99f});
	m_sensor.set_position(get_global_center());
	m_textured = false;
}

AmbientProp::AmbientProp(automa::ServiceProvider& svc, int channel, std::string_view tag) : Entity{svc, "ambient_props", 0}, m_tag{tag.data()}, m_channel{channel} { m_textured = false; }

std::unique_ptr<Entity> AmbientProp::clone() const { return std::make_unique<AmbientProp>(*this); }

void AmbientProp::serialize(dj::Json& out) {
	Entity::serialize(out);
	out["tag"] = m_tag;
	out["channel"] = m_channel;
	out["foreground"] = is_foreground();
}

void AmbientProp::unserialize(dj::Json const& in) {
	Entity::unserialize(in);
	m_tag = in["tag"].as_string();
	m_channel = in["channel"].as<int>();
}

void AmbientProp::expose() { Entity::expose(); }

void AmbientProp::update(automa::ServiceProvider& svc, world::Map& map, SceneContext& context, player::Player& player) {
	Entity::update(svc, map, context, player);
	if (m_params) {
		if (m_sensor.within_bounds(player.hurtbox)) {
			auto pvel = player.get_collider().physics.actual_velocity() * 0.0008f;
			m_bob.physics.apply_force(pvel);
			NANI_LOG_DEBUG(m_logger, "{}", m_bob.physics.position.x);
		}
		m_bob.steering.seek(m_bob.physics, {}, m_params->sensitivity);
		m_bob.physics.simple_update();

		auto displacement = m_bob.physics.position.x;
		float normalized = std::tanh(displacement);
		auto frame = util::map_to_frame(normalized, -1.0f, 1.0f, 0, m_params->num_frames - 1);
		set_frame(frame);
		set_channel(m_channel);
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
			Animatable::set_position(get_global_center() + cam + m_params->offset);
			win.draw(*this);
		}
		return;
	}
	if (m_params) {
		Animatable::set_position(get_global_center() - cam + m_params->offset);
		win.draw(*this);
	}

	/*sf::CircleShape bob{};
	bob.setFillColor(sf::Color::Red);
	bob.setRadius(2.f);
	bob.setPosition(get_global_center() - cam + m_bob.physics.position);
	win.draw(bob);
	m_sensor.render(win, cam);*/
}

AmbientPropParameters::AmbientPropParameters(dj::Json const& in) {
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
}
} // namespace fornani
