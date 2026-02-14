
#include <fornani/physics/ICollider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::shape {

ICollider::ICollider(sf::Vector2f dimensions) : p_vicinity{dimensions + sf::Vector2f{vicinity_pad_v * 2.f, vicinity_pad_v * 2.f}} {}

void ICollider::handle_collision(ICollider& other) {}

void ICollider::update(automa::ServiceProvider& svc) {
	auto dim = p_vicinity.get_dimensions() - sf::Vector2f{vicinity_pad_v * 2.f, vicinity_pad_v * 2.f};
	auto center = is(ColliderType::circle) ? physics.position : physics.position + dim * 0.5f;
	p_vicinity.set_position(center - p_vicinity.get_dimensions() * 0.5f);
	set_flag(ColliderFlags::changed, false);
	if (physics.actual_speed() > constants::tiny_value) { set_flag(ColliderFlags::changed); }

	// some colliders may have custom physics properties (like the player) that must be unserialized
	// colliders will not have water physics by default (for now). TODO: change that
	if (!has_flag_set(ColliderFlags::custom_properties)) { return; }

	auto v = std::abs(physics.velocity.y);
	auto v_min = 0.f;
	auto v_max = 100.f;
	auto result = std::lerp(p_physics_properties.water_surface_multiplier, 0.5f, std::clamp((v - v_min) / (v_max - v_min), 0.0f, 1.0f));
	if (has_flag_set(ColliderFlags::in_water) && !has_flag_set(ColliderFlags::submerged)) { physics.acceleration.y *= result; }

	physics.set_constant_friction(p_physics_properties.friction);
	physics.gravity = p_physics_properties.gravity;
	if (has_flag_set(ColliderFlags::in_water)) {
		auto multiplier = has_flag_set(ColliderFlags::submerged) ? p_physics_properties.water_gravity_multiplier : 0.9f;
		if (has_flag_set(ColliderFlags::sinking)) { multiplier = 0.005f; }
		physics.set_constant_friction(p_physics_properties.friction * p_physics_properties.water_friction_multiplier);
		physics.gravity = p_physics_properties.gravity * multiplier;
	}
}

void ICollider::handle_map_collision(world::Map& map) {}

void ICollider::handle_map_collision(world::Tile const& tile) {}

void ICollider::detect_map_collision(world::Map& map) {}

void ICollider::handle_collision(Shape const& shape, bool soft) {}

bool ICollider::handle_collider_collision(Shape const& collider, bool soft, sf::Vector2f velocity, float force, bool crusher) { return false; }

void ICollider::handle_collider_collision(Collider const& collider, bool momentum) {}

void ICollider::handle_collider_collision(CircleCollider& collider) {}

void ICollider::render(sf::RenderWindow& win, sf::Vector2f cam) { p_vicinity.render(win, cam, sf::Color{80, 80, 10, 10}); }

void ICollider::load_properties(dj::Json const& in) {
	p_physics_properties.unserialize(in);
	set_flag(ColliderFlags::custom_properties);
}

void ICollider::register_chunks(world::Map& map) {
	if (!was_changed()) { return; }
	auto old_chunks = m_chunks;
	m_chunks.clear();
	for (auto [i, vertex] : std::views::enumerate(p_vicinity.vertices)) { m_chunks.add(map.get_chunk_id_from_position(p_vicinity.vertices[i])); }
	std::sort(old_chunks.begin(), old_chunks.end());
	std::sort(m_chunks.begin(), m_chunks.end());
	if (old_chunks != m_chunks) { map.refresh_collider_chunks(old_chunks, m_chunks, this); }
}

bool ICollider::should_exclude(ICollider const& other) const { return m_exclusion_targets.any(other.get_traits()); }

bool ICollider::should_exclude_resolution_with(ICollider const& other) const { return m_resolution_exclusion_targets.any(other.get_traits()); }

bool ICollider::should_softly_collide_with(ICollider const& other) const { return m_soft_targets.any(other.get_traits()); }

std::vector<int> ICollider::compute_chunks(world::Map& map) {
	m_chunks.clear();
	for (auto [i, vertex] : std::views::enumerate(p_vicinity.vertices)) { m_chunks.add(map.get_chunk_id_from_position(p_vicinity.vertices[i])); }
	return get_chunks();
}

std::vector<int> ICollider::get_chunks() const {
	auto ret = std::vector<int>{};
	for (auto const& entry : m_chunks) { ret.push_back(static_cast<int>(entry)); }
	return ret;
}

std::vector<std::string> ICollider::print_chunks() {
	auto ret = std::vector<std::string>{};
	for (auto const& entry : m_chunks) { ret.push_back(std::to_string(static_cast<int>(entry))); }
	return ret;
}

} // namespace fornani::shape
