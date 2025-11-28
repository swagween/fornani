#include "fornani/world/Pushable.hpp"

#include <cmath>
#include "fornani/entities/player/Player.hpp"
#include "fornani/particle/Effect.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Constants.hpp"
#include "fornani/world/Map.hpp"

#include "fornani/utils/Random.hpp"

#include <ccmath/ext/clamp.hpp>

namespace fornani::world {

Pushable::Pushable(automa::ServiceProvider& svc, sf::Vector2f position, int style, int size)
	: Drawable{svc, "pushables"}, style(style), size(size), collider{constants::f_cell_vec * static_cast<float>(size) - sf::Vector2f{1.f, 0.1f}}, collision_box{constants::f_cell_vec * static_cast<float>(size)}, speed{1.f} {
	collider.physics.position = position;
	start_position = position;
	collider.physics.set_friction_componentwise({0.97f, 0.99f});
	collider.stats.GRAV = 18.0f;
	mass = static_cast<float>(size);
	collider.sync_components();
	auto lock = collider.snap_to_grid(static_cast<float>(size));
	collider.physics.position = lock;
	start_box = collider.bounding_box;
	set_texture_rect(sf::IntRect{{style * 2 * constants::i_cell_resolution, (size - 1) * constants::i_cell_resolution}, constants::i_resolution_vec * size});
	collider.flags.general.reset(shape::General::complex);
	collider.vertical_detector_buffer = 0.1f;
	collider.horizontal_detector_buffer = 0.1f;
	collider.wallslide_pad = 1.f;
}

void Pushable::update(automa::ServiceProvider& svc, Map& map, player::Player& player) {
	collision_box.set_position(collider.physics.position - sf::Vector2f{0.f, 1.f});
	energy = ccm::ext::clamp(energy - dampen, 0.f, std::numeric_limits<float>::max());
	if (energy < 0.2f) { energy = 0.f; }
	if (svc.ticker.every_x_ticks(20)) { random_offset = random::random_vector_float(-energy, energy); }
	weakened.update();
	if (weakened.is_complete()) { hit_count.start(); }
	player.on_crush(map);
	for (auto& enemy : map.enemy_catalog.enemies) { enemy->on_crush(map); }

	// reset position if it's far away, and if the player isn't overlapping the start position
	if (hit_count.get_count() > 2 || map.off_the_bottom(collider.physics.position)) {
		bool can_respawn = true;
		if (player.collider.bounding_box.overlaps(start_box)) { can_respawn = false; }
		for (auto& pushable : map.pushables) {
			if (pushable.get_bounding_box().overlaps(start_box) && &pushable != this) { can_respawn = false; }
		}
		if (can_respawn) {
			reset(svc, map);
			svc.soundboard.flags.world.set(audio::World::small_crash);
		}
		hit_count.start();
	}

	// player pushes block
	state.reset(PushableState::pushed);
	if (player.collider.wallslider.overlaps(collision_box) && player.pushing() && player.is_in_animation(player::AnimState::push) && collider.physics.actual_velocity().y < 1.0f) {
		if (player.controller.moving_left() && player.collider.physics.position.x > collider.physics.position.x) { collider.physics.acceleration.x = -speed / mass; }
		if (player.controller.moving_right() && player.collider.physics.position.x < collider.physics.position.x) { collider.physics.acceleration.x = speed / mass; }
		if (ccm::abs(collider.physics.actual_velocity().x) > constants::small_value) { svc.soundboard.flags.world.set(audio::World::pushable_move); }
		state.set(PushableState::moved);
		state.set(PushableState::pushed);
	}

	for (auto& pushable : map.pushables) {
		if (&pushable == this) { continue; }
		if (!pushable.collider.vicinity.overlaps(collider.vicinity)) { continue; }
		if (collider.jumpbox.overlaps(pushable.collider.predictive_vertical) && !is_being_pushed()) { collider.physics.adopt(pushable.collider.physics); }
		if (!is_being_pushed() || collider.get_bottom().y < pushable.collider.get_center().y) { collider.handle_collider_collision(pushable.get_bounding_box()); }
	}

	// pushable should only be moved by a platform if it's on top of one

	player.collider.handle_collider_collision(collider.bounding_box);
	if (size == 1) { collider.handle_collider_collision(player.hurtbox); } // big ones should crush the player
	if (player.collider.jumpbox.overlaps(collider.bounding_box) && collider.grounded() && collider.physics.is_moving_horizontally(constants::tiny_value)) { player.collider.physics.forced_momentum = collider.physics.forced_momentum; }

	player.on_crush(map);
	for (auto& enemy : map.enemy_catalog.enemies) {
		if (enemy->has_map_collision()) { enemy->get_collider().handle_collider_collision(collider); }
		enemy->on_crush(map);
	}

	auto test_position = is_moving() ? collider.get_center() : collider.get_bottom();
	for (auto const& it : map.breakable_iterators[map.get_chunk_id_from_position(collider.get_bottom())]) { collider.handle_collider_collision(it->get_bounding_box()); }
	for (auto& block : map.switch_blocks) {
		if (block.on()) { collider.handle_collider_collision(block.get_bounding_box()); }
	}
}

void Pushable::post_update(automa::ServiceProvider& svc, Map& map, player::Player& player) {
	for (auto& pushable : map.pushables) {
		if (&pushable == this) { continue; }
		if (!pushable.collider.vicinity.overlaps(collider.vicinity)) { continue; }
	}

	collider.physics.impart_momentum();
	collider.update(svc);

	for (auto& pushable : map.pushables) {
		if (&pushable == this) { continue; }
		if (!pushable.collider.vicinity.overlaps(collider.vicinity)) { continue; }
		if (pushable.collider.wallslider.overlaps(collision_box)) {
			if (pushable.collider.pushes(collider) && collider.grounded()) { collider.physics.adopt(pushable.collider.physics); }
		}
	}

	collider.detect_map_collision(map);
	for (auto& platform : map.platforms) {
		if (platform.bounding_box.overlaps(collider.jumpbox)) { collider.handle_collider_collision(platform.bounding_box); }
		if (collider.jumpbox.overlaps(platform.bounding_box) && !collider.perma_grounded() && platform.is_sticky()) {
			collider.physics.forced_momentum = platform.physics.apparent_velocity();
			if (player.collider.jumpbox.overlaps(collision_box) && !player.collider.perma_grounded() && !(player.collider.has_left_wallslide_collision() || player.collider.has_right_wallslide_collision())) {
				player.collider.physics.forced_momentum = collider.physics.forced_momentum;
			}
		}
	}

	if (collider.flags.state.test(shape::State::just_landed)) {
		map.effects.push_back(entity::Effect(svc, "dust", collider.get_center()));
		svc.soundboard.flags.world.set(audio::World::thud);
	}

	collider.reset();
	collider.reset_ground_flags();
	collider.physics.acceleration = {};

	if (!collider.has_jump_collision()) { collider.physics.forced_momentum = {}; }
	if (collider.has_left_wallslide_collision() || collider.has_right_wallslide_collision() || collider.flags.external_state.test(shape::ExternalState::vert_world_collision) || collider.world_grounded()) {
		collider.physics.forced_momentum = {};
	}
}

void Pushable::handle_collision(shape::Collider& other) const { other.handle_collider_collision(collider); }

void Pushable::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Drawable::set_position(collision_box.get_position() - cam + random_offset);
	if (svc.greyblock_mode()) {
		collider.render(win, cam);
		sf::RectangleShape box{};
		box.setSize(start_box.get_dimensions());
		box.setFillColor(sf::Color::Transparent);
		is_moving() ? box.setOutlineColor(sf::Color::Green) : box.setOutlineColor(sf::Color::Red);
		box.setOutlineThickness(-1);
		box.setPosition(start_box.get_position() - cam);
		// win.draw(box);
		sf::RectangleShape coll{};
		coll.setSize(collision_box.get_dimensions());
		coll.setFillColor(sf::Color::Transparent);
		coll.setOutlineThickness(-2.f);
		is_being_pushed() ? coll.setOutlineColor(colors::mythic_green) : is_moving() ? coll.setOutlineColor(colors::goldenrod) : coll.setOutlineColor(colors::dark_goldenrod);
		coll.setPosition(collision_box.get_position() - cam);
		win.draw(coll);
	} else {
		win.draw(*this);
	}
}

void Pushable::on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj) {
	if (proj.transcendent()) { return; }
	if (proj.get_collider().collides_with(collider.bounding_box)) {
		hit_count.update();
		weakened.start();
		if (!proj.destruction_initiated()) {
			energy = hit_energy;
			svc.soundboard.flags.world.set(audio::World::breakable_hit);
		}
		proj.destroy(false);
	}
}

void Pushable::reset(automa::ServiceProvider& svc, world::Map& map) {
	auto label = size == 1 ? "small_explosion" : "large_explosion";
	map.effects.push_back(entity::Effect(svc, label, collider.get_center()));
	collider.physics.position = start_position;
	map.effects.push_back(entity::Effect(svc, label, collider.get_center()));
}

} // namespace fornani::world
