
#include <fornani/core/Debug.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/entity/SpecialBlock.hpp>

namespace fornani {

SpecialBlock::SpecialBlock(automa::ServiceProvider& svc, dj::Json const& in) : Entity(svc, in, "special_blocks") {
	unserialize(in);
	m_textured = false;
	owned_collider.reset();
	collider.reset();
	copyable = false;
}

SpecialBlock::SpecialBlock(automa::ServiceProvider& svc, world::Map& map, dj::Json const& in) : Entity(svc, in, "special_blocks") {
	owned_collider.emplace(map, sf::Vector2f{constants::f_cell_vec - sf::Vector2f{constants::tiny_value, constants::tiny_value}});
	collider = *owned_collider;
	unserialize(in);
	m_textured = false;
	copyable = false;
}

SpecialBlock::SpecialBlock(automa::ServiceProvider& svc, sf::Vector2u dimensions, int id, std::string_view label, util::BitFlags<SpecialBlockAttributes> attributes) : Entity(svc, "special_blocks", id, dimensions), m_attributes{attributes} {
	m_textured = false;
	owned_collider.reset();
	collider.reset();
	copyable = false;
}

void SpecialBlock::serialize(dj::Json& out) {
	Entity::serialize(out);
	out["type"] = get_type();
	out["wallslide"] = m_attributes.test(SpecialBlockAttributes::wallslide);
}

void SpecialBlock::unserialize(dj::Json const& in) {
	Entity::unserialize(in);

	if (in["wallslide"].as_bool()) {
		m_attributes.set(SpecialBlockAttributes::wallslide);
	} else {
		m_attributes.reset(SpecialBlockAttributes::wallslide);
	}
}

void SpecialBlock::update(automa::ServiceProvider& svc, world::Map& map, SceneContext& context, player::Player& player) { Entity::update(svc, map, context, player); }

void SpecialBlock::init(automa::ServiceProvider& svc, dj::Json const& in) {
	if (!has_collider()) { return; }
	get_collider().set_position(get_world_position());
	get_collider().set_trait(shape::ColliderTrait::block);
	get_collider().set_attribute(shape::ColliderAttributes::fixed);
}

void SpecialBlock::on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj) {
	if (!has_collider()) { return; }
	if (ignore_updates()) { return; }
	if (proj.transcendent()) { return; }
	if (proj.reflect()) {
		proj.get_collider().handle_collision(get_collider().bounding_box);
		return;
	}
	if (proj.get_collider().collides_with(get_collider().bounding_box)) { proj.handle_hard_hit(svc, map); }
}

auto SpecialBlock::ignore_updates() const -> bool { return false; }

void SpecialBlock::expose() { Entity::expose(); }

void SpecialBlock::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	Entity::render(win, cam, size);
	if (debug::is_production()) { return; }
	if (!m_editor) { return; }
}

} // namespace fornani
