
#include <fornani/entities/player/Player.hpp>
#include <fornani/entity/Water.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

Water::Water(automa::ServiceProvider& svc, dj::Json const& in) : Entity(svc, in, "water"), m_bounding_box{get_world_dimensions()}, m_replenish_cooldown{260}, m_mode{sf::BlendNone} {
	unserialize(in);
	p_animatable.set_channel(get_i_type());
	auto lookup = sf::Vector2i{66 * static_cast<int>(m_type), 0};
	auto buffer = sf::Vector2i{1, 1};
	repeatable = false;
	copyable = false;

	auto pos = get_world_position().length() < constants::small_value ? -constants::f_cell_vec : get_world_position();
	m_bounding_box.set_position(pos);

	auto u_dimensions = sf::Vector2u{p_animatable.get_dimensions()};
	if (!m_texture.resize(u_dimensions)) { NANI_LOG_WARN(m_logger, "Failed to resize map texture"); }
	if (!m_surface_texture.resize(sf::Vector2u{u_dimensions.x, 64})) { NANI_LOG_WARN(m_logger, "Failed to resize map texture"); }
	m_surface_texture.clear(sf::Color::Transparent);

	auto state = sf::RenderStates{m_mode};

	auto wat = sf::Sprite{svc.assets.get_texture("water")};
	auto spr = sf::Sprite{svc.assets.get_texture("water_surface")};
	wat.setTextureRect(sf::IntRect{lookup + buffer, constants::i_resolution_vec * 4});
	spr.setTextureRect(sf::IntRect{lookup + buffer, constants::i_resolution_vec * 4});
	m_texture.draw(wat, state);
	m_surface_texture.draw(spr, state);

	m_texture.display();
	m_surface_texture.display();
	m_texture.setRepeated(true);
	m_surface_texture.setRepeated(true);

	if (m_type == WaterType::curative) {
		m_sparkler.emplace(svc, get_world_dimensions(), colors::nani_white, "curative_water");
		m_sparkler->set_position(get_world_position());
	}
}

Water::Water(automa::ServiceProvider& svc, sf::Vector2u dimensions, int id, WaterType type) : Entity(svc, "water", id, dimensions), m_type{type} {
	repeatable = false;
	copyable = false;
}

void Water::serialize(dj::Json& out) {
	Entity::serialize(out);
	out["type"] = static_cast<int>(m_type);
}

void Water::unserialize(dj::Json const& in) {
	Entity::unserialize(in);
	m_type = static_cast<WaterType>(in["type"].as<int>());
}

void Water::expose() {
	Entity::expose();
	static int type{};
	ImGui::InputInt("Type", &type);
	m_type = static_cast<WaterType>(type);
}

void Water::update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] SceneContext& context, [[maybe_unused]] player::Player& player) {
	m_replenish_cooldown.update();
	if (m_sparkler) { m_sparkler->update(svc); }
	if (m_bounding_box.contains_point(player.get_collider().get_bottom())) {
		if (!has_flag_set(WaterFlags::touched)) {
			m_replenish_cooldown.start();
			set_flag(WaterFlags::touched);
		}
		if (m_type == WaterType::curative) {
			if (m_replenish_cooldown.is_almost_complete()) {
				m_replenish_cooldown.start();
				player.heal();
			}
			svc.soundboard.repeat_sound("regenerate");
		}
		if (m_type == WaterType::toxic) { player.hurt(); }
	} else {
		set_flag(WaterFlags::touched, false);
		m_replenish_cooldown.cancel();
	}
	if (m_bounding_box.contains_point(player.hurtbox.get_center())) {
		if (!has_flag_set(WaterFlags::splashed)) {
			svc.soundboard.flags.world.set(audio::World::splash);
			map.spawn_effect(svc, "splash", sf::Vector2f{player.get_collider().get_center().x, m_bounding_box.get_position().y}, {}, get_i_type());
		}
		player.get_collider().set_flag(shape::ColliderFlags::in_water);
		set_flag(WaterFlags::splashed);
	} else {
		set_flag(WaterFlags::splashed, false);
	}
	if (m_bounding_box.contains_point(player.get_collider().get_top() + sf::Vector2f{0.f, -2.f})) {
		player.get_collider().set_flag(shape::ColliderFlags::submerged);
	} else if (player.get_collider().has_flag_set(shape::ColliderFlags::in_water)) {
		player.exit_water();
	}

	m_wave_timer.update(0.005f);
	auto phase = m_wave_timer.get();
	auto wrapped = std::fmod(phase, 2.f * std::numbers::pi);
	m_wave_shift.x = std::cos(wrapped);
	m_wave_shift.y = std::sin(2.f * wrapped);
	if (phase >= 2.f * std::numbers::pi) { m_wave_timer.reset(); }
	auto pos = get_world_position() == sf::Vector2f{0.f, 0.f} ? -constants::f_cell_vec : get_world_position();
	m_bounding_box.set_position(pos);
	m_bounding_box.set_position({m_bounding_box.get_position().x + m_wave_shift.x * 3.f, m_bounding_box.get_position().y + m_wave_shift.y * 2.f});
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
	auto sprite = sf::Sprite{m_texture.getTexture()};
	sprite.setTextureRect(sf::IntRect{{}, sf::Vector2i{get_world_dimensions() * 0.5f}});
	sprite.setScale(constants::f_scale_vec);
	sprite.setPosition(m_bounding_box.get_position() - cam + sf::Vector2f{0.f, 2.f} + m_wave_shift);
	win.draw(sprite);
	auto surface_sprite = sf::Sprite{m_surface_texture.getTexture()};
	surface_sprite.setTextureRect(sf::IntRect{{}, sf::Vector2i{static_cast<int>(get_world_dimensions().x * 0.5f), 32}});
	surface_sprite.setScale(constants::f_scale_vec);
	surface_sprite.setPosition(m_bounding_box.get_position() + m_wave_shift - cam);
	win.draw(surface_sprite);
	if (m_sparkler) { m_sparkler->render(win, cam); }
}

} // namespace fornani
