
#include <fornani/core/Debug.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/entity/BoxCollider.hpp>

namespace fornani {

BoxCollider::BoxCollider(automa::ServiceProvider& svc, dj::Json const& in) : Entity(svc, in, "box_colliders"), m_collider{get_world_dimensions()} {
	unserialize(in);
	m_textured = false;
	m_collider.set_trait(shape::ColliderTrait::block);
	m_collider.set_attribute(shape::ColliderAttributes::fixed);
	m_collider.set_attribute(shape::ColliderAttributes::no_wallslide);
	m_collider.set_position(get_world_position());
}

BoxCollider::BoxCollider(automa::ServiceProvider& svc, sf::Vector2u dimensions, int id, util::BitFlags<BoxColliderAttributes> attributes) : Entity(svc, "box_colliders", id, dimensions), m_collider{sf::Vector2f{dimensions}} {
	m_textured = false;
}

std::unique_ptr<Entity> BoxCollider::clone() const { return std::make_unique<BoxCollider>(*this); }

void BoxCollider::serialize(dj::Json& out) {
	Entity::serialize(out);
	out["wallslide"] = m_attributes.test(BoxColliderAttributes::wallslide);
}

void BoxCollider::unserialize(dj::Json const& in) {
	Entity::unserialize(in);
	if (in["wallslide"].as_bool()) { m_attributes.set(BoxColliderAttributes::wallslide); }
}

void BoxCollider::update(automa::ServiceProvider& svc, world::Map& map, SceneContext& context, player::Player& player) {
	Entity::update(svc, map, context, player);
	m_collider.update(svc);
	m_collider.handle_collider_collision(player.get_collider());
	player.get_collider().handle_collider_collision(m_collider);
}

void BoxCollider::expose() { Entity::expose(); }

void BoxCollider::render(sf::RenderWindow& win, sf::Vector2f cam, float size) {
	Entity::render(win, cam, size);
	if (debug::is_production()) { return; }
	m_collider.render(win, cam);
	if (!m_editor) { return; }
}

} // namespace fornani
