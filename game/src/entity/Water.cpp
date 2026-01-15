
#include <fornani/entities/player/Player.hpp>
#include <fornani/entity/Water.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

Water::Water(automa::ServiceProvider& svc, dj::Json const& in) : Entity(svc, in, "water"), m_bounding_box{get_world_dimensions()} {
	unserialize(in);
	m_bounding_box.set_position(get_world_position());
	set_texture_rect(sf::IntRect{{}, constants::i_resolution_vec});
	repeatable = false;
}

Water::Water(automa::ServiceProvider& svc, sf::Vector2u dimensions, int id, bool toxic) : Entity(svc, "water", id, dimensions) {
	set_flag(WaterFlags::toxic, toxic);
	set_texture_rect(sf::IntRect{{}, constants::i_resolution_vec});
	repeatable = false;
}

std::unique_ptr<Entity> Water::clone() const { return std::make_unique<Water>(*this); }

void Water::serialize(dj::Json& out) {
	Entity::serialize(out);
	out["toxic"] = has_flag_set(WaterFlags::toxic);
}

void Water::unserialize(dj::Json const& in) {
	Entity::unserialize(in);
	set_flag(WaterFlags::toxic, in["toxic"].as_bool());
}

void Water::expose() {
	static bool toxic{};
	ImGui::Checkbox("Toxic?", &toxic);
	set_flag(WaterFlags::toxic, toxic);
}

void Water::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	highlighted ? drawbox.setFillColor(sf::Color{60, 255, 120, 180}) : drawbox.setFillColor(sf::Color{60, 255, 120, 80});
	Entity::render(win, cam, size);
	if (!m_editor) {
		drawbox.setSize(get_world_dimensions() * size);
		drawbox.setPosition(get_world_dimensions() * size + cam);
		win.draw(drawbox);
	}
	m_bounding_box.render(win, cam, sf::Color{60, 60, 200, 100});
}

void Water::update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] std::optional<std::unique_ptr<gui::Console>>& console, [[maybe_unused]] player::Player& player) {
	m_bounding_box.set_position(get_world_position());
	if (player.get_collider().hurtbox.overlaps(m_bounding_box)) {
		if (!player.has_flag_set(player::PlayerFlags::in_water)) { svc.soundboard.flags.world.set(audio::World::splash); }
		player.set_flag(player::PlayerFlags::in_water);
	} else {
		player.set_flag(player::PlayerFlags::in_water, false);
	}
	player.set_flag(player::PlayerFlags::submerged, m_bounding_box.contains_point(player.get_collider().get_top()));
}

} // namespace fornani
