
#include <fornani/entities/player/Player.hpp>
#include <fornani/entity/Water.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

Water::Water(automa::ServiceProvider& svc, dj::Json const& in) : Entity(svc, in, "water"), m_bounding_box{get_world_dimensions()}, m_surface{svc, "water_surface"} {
	unserialize(in);
	m_bounding_box.set_position(get_world_position());
	set_texture_rect(sf::IntRect{{}, constants::i_resolution_vec * 4});
	repeatable = false;
	copyable = false;

	auto u_dimensions = sf::Vector2u{get_dimensions() * 4};
	if (!m_texture.resize(u_dimensions)) { NANI_LOG_WARN(m_logger, "Failed to resize map texture"); }
	if (!m_surface_texture.resize(sf::Vector2u{u_dimensions.x, 64})) { NANI_LOG_WARN(m_logger, "Failed to resize map texture"); }
	m_texture.draw(get_sprite());
	m_surface_texture.draw(m_surface);
	m_texture.display();
	m_surface_texture.display();
	m_texture.setRepeated(true);
	m_surface_texture.setRepeated(true);

	m_bounding_box.set_position(get_world_position());
}

Water::Water(automa::ServiceProvider& svc, sf::Vector2u dimensions, int id, bool toxic) : Entity(svc, "water", id, dimensions), m_surface{svc, "water_surface"} {
	toxic ? m_attributes.set(WaterAttributes::toxic) : m_attributes.reset(WaterAttributes::toxic);
	set_texture_rect(sf::IntRect{{}, constants::i_resolution_vec});
	repeatable = false;
	copyable = false;
}

void Water::serialize(dj::Json& out) {
	Entity::serialize(out);
	out["toxic"] = m_attributes.test(WaterAttributes::toxic);
}

void Water::unserialize(dj::Json const& in) {
	Entity::unserialize(in);
	in["toxic"].as_bool() ? m_attributes.set(WaterAttributes::toxic) : m_attributes.reset(WaterAttributes::toxic);
}

void Water::expose() {
	Entity::expose();
	static bool toxic{};
	ImGui::Checkbox("Toxic?", &toxic);
	toxic ? m_attributes.set(WaterAttributes::toxic) : m_attributes.reset(WaterAttributes::toxic);
}

void Water::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	highlighted ? drawbox.setFillColor(sf::Color{60, 255, 120, 180}) : drawbox.setFillColor(sf::Color{60, 255, 120, 80});
	Entity::render(win, cam, size);
	if (!m_editor) {
		drawbox.setSize(get_world_dimensions() * size);
		drawbox.setPosition(get_world_dimensions() * size + cam);
		// win.draw(drawbox);
	}
	if (m_editor) { return; }
	m_bounding_box.render(win, cam + sf::Vector2f{0.f, -2.f}, sf::Color{100, 160, 220, 30});
	auto sprite = sf::Sprite{m_texture.getTexture()};
	sprite.setTextureRect(sf::IntRect{{}, sf::Vector2i{get_world_dimensions()}});
	sprite.setPosition(m_bounding_box.get_position() - cam);
	win.draw(sprite);
	auto surface_sprite = sf::Sprite{m_surface_texture.getTexture()};
	surface_sprite.setTextureRect(sf::IntRect{{}, sf::Vector2i{static_cast<int>(get_world_dimensions().x), 32}});
	surface_sprite.setPosition(m_bounding_box.get_position() + m_wave_shift - cam);
	win.draw(surface_sprite);
}

void Water::update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] std::optional<std::unique_ptr<gui::Console>>& console, [[maybe_unused]] player::Player& player) {
	if (player.get_collider().hurtbox.overlaps(m_bounding_box)) {
		if (!has_flag_set(WaterFlags::splashed)) {
			svc.soundboard.flags.world.set(audio::World::splash);
			map.spawn_effect(svc, "splash", sf::Vector2f{player.get_collider().get_center().x, m_bounding_box.get_position().y});
		}
		player.get_collider().set_flag(shape::ColliderFlags::in_water);
		if (m_attributes.test(WaterAttributes::toxic)) {
			if (svc.ticker.every_x_ticks(256)) { player.hurt(); }
		}
		set_flag(WaterFlags::splashed);
	} else {
		set_flag(WaterFlags::splashed, false);
	}
	if (m_bounding_box.contains_point(player.get_collider().get_top())) { player.get_collider().set_flag(shape::ColliderFlags::submerged); }

	m_wave_timer.update(0.005f);
	auto phase = m_wave_timer.get();
	auto wrapped = std::fmod(phase, 2.f * std::numbers::pi);
	m_wave_shift.x = std::cos(wrapped);
	m_wave_shift.y = std::sin(2.f * wrapped);
	if (phase >= 2.f * std::numbers::pi) { m_wave_timer.reset(); }
	m_bounding_box.set_position(get_world_position());
	m_bounding_box.set_position({m_bounding_box.get_position().x + m_wave_shift.x * 3.f, m_bounding_box.get_position().y + m_wave_shift.y * 2.f});
}

} // namespace fornani
