
#include <fornani/core/Debug.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/entity/Destructible.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

Destructible::Destructible(automa::ServiceProvider& svc, dj::Json const& in) : Entity(svc, in, "destructibles") {
	unserialize(in);
	p_animatable.set_texture_rect(sf::IntRect{{}, constants::i_resolution_vec});
	repeatable = false;
	copyable = false;
	owned_collider.reset();
	collider.reset();
}

Destructible::Destructible(automa::ServiceProvider& svc, world::Map& map, dj::Json const& in) : Destructible(svc, in) {
	owned_collider.emplace(map, sf::Vector2f{constants::f_cell_vec - sf::Vector2f{2.f, 2.f}});
	collider = *owned_collider;
	init(svc, in);
	p_animatable.set_texture_rect(sf::IntRect{{map.get_style_id() * constants::i_cell_resolution, 0}, constants::i_resolution_vec});
}

Destructible::Destructible(automa::ServiceProvider& svc, int id, util::BitFlags<DestructibleAttributes> attributes) : Entity(svc, "destructibles", id), m_attributes{attributes} {
	p_animatable.set_texture_rect(sf::IntRect{{}, constants::i_resolution_vec});
	repeatable = false;
	copyable = false;
	owned_collider.reset();
	collider.reset();
}

void Destructible::serialize(dj::Json& out) {
	Entity::serialize(out);
	out["unlit"] = m_attributes.test(DestructibleAttributes::unlit);
	out["inverse"] = m_attributes.test(DestructibleAttributes::inverse);
	out["enemy_clear"] = m_attributes.test(DestructibleAttributes::enemy_clear);
}

void Destructible::unserialize(dj::Json const& in) {
	Entity::unserialize(in);
	repeatable = false;
	copyable = false;
	quest_id = in["id"].as<int>();
	in["unlit"].as_bool() ? m_attributes.set(DestructibleAttributes::unlit) : m_attributes.reset(DestructibleAttributes::unlit);
	in["inverse"].as_bool() ? m_attributes.set(DestructibleAttributes::inverse) : m_attributes.reset(DestructibleAttributes::inverse);
	in["enemy_clear"].as_bool() ? m_attributes.set(DestructibleAttributes::enemy_clear) : m_attributes.reset(DestructibleAttributes::enemy_clear);
}

void Destructible::expose() { Entity::expose(); }

auto Destructible::ignore_updates() const -> bool { return is_destroyed(); }

void Destructible::debug() {
	static auto sz = ImVec2{180.f, 450.f};
	ImGui::SetNextWindowSize(sz);
	if (ImGui::Begin("Destructible Debug")) {
		ImGui::SeparatorText("Info");
		ImGui::Text("State: %s", m_state == 0 ? "Solid" : m_state == 1 ? "Destroyed" : "ERROR");
		ImGui::Text("Code: %i", quest_id);
		ImGui::Text("Ignore Updates?: %s", ignore_updates() ? "Yes" : "No");
		ImGui::Text("Has Collider?: %s", has_collider() ? "Yes" : "No");
		if (has_collider()) {
			ImGui::Text("No Collision?: %s", get_collider().has_attribute(shape::ColliderAttributes::no_collision) ? "Yes" : "No");
			ImGui::Text("Intangible?: %s", get_collider().has_flag_set(shape::ColliderFlags::intangible) ? "Yes" : "No");
		}
		ImGui::End();
	}
}

void Destructible::update(automa::ServiceProvider& svc, world::Map& map, SceneContext& context, player::Player& player) {
	if (!has_collider()) { return; }
	get_collider().set_attribute(shape::ColliderAttributes::no_collision, ignore_updates());
	get_collider().set_flag(shape::ColliderFlags::intangible, ignore_updates());
	auto state = svc.data.get_destructible_state(quest_id);
	if (state != m_state && state != -1) {
		m_state = state;
		map.effects.push_back(entity::Effect(svc, "small_explosion", get_global_center()));
		svc.soundboard.flags.world.set(audio::World::block_toggle);
	}
	if (map.enemies_cleared() && is_enemy_clear() && m_state == 0) { svc.data.switch_destructible_state(quest_id); }
	if (ignore_updates()) { return; }
	player.get_collider().handle_collider_collision(get_collider());
}

void Destructible::init(automa::ServiceProvider& svc, dj::Json const& in) {
	auto state = svc.data.get_destructible_state(quest_id);
	m_state = state != -1 ? state : 0;
	m_state = in["inverse"].as_bool() ? 1 : m_state;
	if (!has_collider()) { return; }
	get_collider().set_position(get_world_position());
	get_collider().set_trait(shape::ColliderTrait::block);
	get_collider().set_attribute(shape::ColliderAttributes::fixed);
}

void Destructible::on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj) {
	if (!has_collider()) { return; }
	if (ignore_updates()) { return; }
	if (proj.transcendent()) { return; }
	if (proj.reflect()) {
		proj.get_collider().handle_collision(get_collider().bounding_box);
		return;
	}
	if (proj.get_collider().collides_with(get_collider().bounding_box)) { proj.handle_hard_hit(svc, map); }
}

shape::Shape& Destructible::get_bounding_box() { return get_collider().bounding_box; }

void Destructible::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	// debug();
	if (ignore_updates()) { return; }
	highlighted ? drawbox.setFillColor(sf::Color{60, 255, 120, 180}) : drawbox.setFillColor(sf::Color{60, 255, 120, 80});
	Entity::render(win, cam, size);
	if (m_editor) { return; }
	p_animatable.set_position(get_world_position() - cam);
	win.draw(p_animatable);
}

void Destructible::render(sf::RenderTexture& tex, sf::Vector2f cam) {
	// debug();
	if (ignore_updates()) { return; }
	p_animatable.set_scale(constants::f_scale_vec);
	p_animatable.set_position(get_world_position());
	tex.draw(p_animatable);
	++debug::draw_calls;
}

} // namespace fornani
