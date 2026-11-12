
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/world/FallingBlock.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::world {

FallingBlock::FallingBlock(automa::ServiceProvider& svc, world::Map& map, sf::Vector2f position) : Animatable{svc, "falling_blocks", {34, 34}}, m_collider{map, 32.f}, m_bounding_box{{64.f, 64.f}} {

	get_collider().set_attribute(shape::ColliderAttributes::no_collision);
	get_collider().set_attribute(shape::ColliderAttributes::no_map_collision);
	get_collider().set_resolution_exclusion_target(shape::ColliderTrait::circle);
	// get_collider().set_resolution_exclusion_target(shape::ColliderTrait::player);
	get_collider().set_exclusion_target(shape::ColliderTrait::circle);
	get_collider().set_exclusion_target(shape::ColliderTrait::player);

	get_collider().physics.set_friction_componentwise({0.998f, 0.998f});
	get_collider().physics.gravity = 6.f;
	get_collider().physics.elasticity = 0.5f;
	get_collider().set_position(position);

	push_and_set_animation("basic", {0, 3, 32, -1});
	center();
}

void FallingBlock::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {

	auto const active = has_flag_set(FallingBlockFlags::emerged) && !has_flag_set(FallingBlockFlags::crashed);
	if (m_bounding_box.overlaps(player.hurtbox) && active) { player.hurt(2.f); }
	set_frame(active ? 1 : 0);

	m_bounding_box.set_position(get_collider().get_global_center() - m_bounding_box.get_dimensions() * 0.5f);

	auto const emerged = !map.overlaps_middleground(m_bounding_box);
	if (emerged && !has_flag_set(FallingBlockFlags::emerged)) {
		set_flag(FallingBlockFlags::emerged);
		get_collider().set_attribute(shape::ColliderAttributes::no_collision, false);
		get_collider().set_attribute(shape::ColliderAttributes::no_map_collision, false);
	}
	if (get_collider().collided() && has_flag_set(FallingBlockFlags::emerged) && !has_flag_set(FallingBlockFlags::crashed)) {
		set_flag(FallingBlockFlags::crashed);
		map.spawn_effect(svc, "strong_slam", get_collider().get_global_center() + sf::Vector2f{0.f, 20.f});
		svc.soundboard.play_sound("clean_impact", get_collider().get_global_center());
		svc.camera_controller.shake(4, 0.1f, 200, 20);
	}
	if (has_flag_set(FallingBlockFlags::crashed)) {
		get_collider().set_attribute(shape::ColliderAttributes::no_collision);
		get_collider().set_attribute(shape::ColliderAttributes::no_map_collision);
	}
	get_collider().physics.acceleration = {};
}

void FallingBlock::render(sf::RenderWindow& win, sf::Vector2f cam) {
	Animatable::set_position(get_collider().get_global_center() - cam);
	win.draw(*this);
	if (debug::is_debug()) {
		sf::RectangleShape box{};
		box.setFillColor(colors::transparent);
		box.setOutlineColor(colors::red);
		box.setOutlineThickness(-1.f);
		box.setPosition(m_bounding_box.get_position() - cam);
		box.setSize(m_bounding_box.get_dimensions());
		win.draw(box);
	}
}

auto FallingBlock::is_gone(world::Map const& map) const -> bool { return get_collider().get_global_center().y > map.real_dimensions.y + 128.f; }

} // namespace fornani::world
