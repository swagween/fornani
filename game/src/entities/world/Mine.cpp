
#include <fornani/core/Debug.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/entities/world/Mine.hpp>
#include <fornani/graphics/Renderer.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::entity {

Mine::Mine(automa::ServiceProvider& svc, world::Map& map, MineType type, int index) : Animatable(svc, "mine", {40, 40}), m_collider(map, 20.f), m_type{type} {
	m_index = static_cast<std::size_t>(index);
	set_frame(random::random_range(0, 1));
	get_collider().set_attribute(shape::ColliderAttributes::no_collision);
	get_collider().set_attribute(shape::ColliderAttributes::custom_resolution);
	get_collider().set_exclusion_target(shape::ColliderTrait::player);
	get_collider().set_exclusion_target(shape::ColliderTrait::npc);
	get_collider().set_exclusion_target(shape::ColliderTrait::enemy);
	get_collider().set_trait(shape::ColliderTrait::particle);
	center();
	get_collider().physics.set_friction_componentwise({0.998f, 0.998f});
}

void Mine::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	if (is_exploded()) { return; }
	if (get_collider().collides_with(player.hurtbox)) { explode(svc, map); }
	for (auto const& e : map.enemy_catalog.enemies) {
		if (get_collider().collides_with(e->get_collider().bounding_box)) { explode(svc, map); }
	}
}

void Mine::render(sf::RenderWindow& win, sf::Vector2f cam) {
	if (is_exploded()) { return; }
	if (debug::is_production()) {
		Animatable::set_position(get_collider().get_global_center() - cam);
		win.draw(*this);
	} else {
		get_collider().render(win, cam);
	}
}

void Mine::submit(Renderer& renderer) {
	auto const pos = get_collider().get_global_center() - Animatable::get_f_dimensions();
	auto const& frame = get_sprite().getTextureRect();

	sf::FloatRect dest{pos, sf::Vector2f{frame.size}};
	renderer.submit(get_sprite().getTexture(), dest, frame, RenderLayer::platforms);
}

void Mine::on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj, player::Player& player) {
	if (is_exploded()) { return; }
	if (get_collider().sensor.within_bounds(proj.get_collider())) {
		auto overlap = get_collider().sensor.get_overlap(proj.get_collider());
		get_collider().physics.velocity = overlap * proj.get_velocity().length();
		proj.handle_hard_hit(svc, map);
	}
}

void Mine::explode(automa::ServiceProvider& svc, world::Map& map) {
	if (is_exploded()) { return; }
	svc.ticker.freeze_frame(0.1f);
	svc.soundboard.play_sound("delay_high", get_collider().get_global_center());
	svc.soundboard.play_sound("deep_pulse", get_collider().get_global_center());
	svc.camera_controller.shake(10, 0.3f, 200, 20);
	m_state = MineState::exploded;
	map.spawn_explosion(svc, "explosion", "explosion", arms::Team::beast, get_collider().get_global_center(), 46.f, 3);
}

} // namespace fornani::entity
