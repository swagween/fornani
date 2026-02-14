
#include <ccmath/ext/clamp.hpp>
#include <fornani/world/Pushable.hpp>
#include <cmath>
#include "fornani/entities/player/Player.hpp"
#include "fornani/particle/Effect.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Constants.hpp"
#include "fornani/utils/Random.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::world {

Pushable::Pushable(automa::ServiceProvider& svc, Map& map, sf::Vector2f position, int style, int size)
	: Drawable{svc, "pushables"}, m_map{&map}, style(style), size(size), m_collider{map, constants::f_cell_vec * static_cast<float>(size) - sf::Vector2f{1.f, 1.f}}, collision_box{constants::f_cell_vec * static_cast<float>(size)},
	  speed{size == 1 ? 1.5f : 1.3f}, m_blink{120}, m_intro{64} {
	get_collider().physics.position = position;
	get_collider().vert_threshold = 0.01f;
	start_position = position;
	m_return_indicator.setSize(get_collider().dimensions);
	m_return_indicator.setOutlineThickness(-4.f);
	m_return_indicator.setFillColor(colors::transparent);
	get_collider().physics.set_friction_componentwise({0.95f, 0.99f});
	get_collider().stats.GRAV = 18.0f;
	mass = static_cast<float>(size);
	get_collider().sync_components();
	auto lock = get_collider().snap_to_grid(static_cast<float>(size));
	get_collider().physics.position = lock;
	start_box = get_collider().bounding_box;
	set_texture_rect(sf::IntRect{{style * 2 * constants::i_cell_resolution, (size - 1) * constants::i_cell_resolution}, constants::i_resolution_vec * size});
	get_collider().flags.general.reset(shape::General::complex);
	get_collider().vertical_detector_buffer = 0.1f;
	get_collider().horizontal_detector_buffer = 0.1f;
	get_collider().wallslide_pad = 1.f;
	get_collider().vert_threshold = 0.1f;

	get_collider().set_attribute(shape::ColliderAttributes::sturdy);
	// get_collider().set_attribute(shape::ColliderAttributes::custom_resolution);
	get_collider().set_trait(shape::ColliderTrait::block);
	get_collider().set_trait(shape::ColliderTrait::pushable);
	get_collider().set_exclusion_target(shape::ColliderTrait::particle);
	get_collider().set_exclusion_target(shape::ColliderTrait::player);
	get_collider().set_exclusion_target(shape::ColliderTrait::enemy);
	get_collider().set_resolution_exclusion_target(shape::ColliderTrait::platform);
	get_collider().set_resolution_exclusion_target(shape::ColliderTrait::npc);
	get_collider().set_exclusion_target(shape::ColliderTrait::npc);
	if (size > 1) { get_collider().set_attribute(shape::ColliderAttributes::crusher); }
	get_collider().wallslide_buffer = 0.9f;
	m_intro.start();
}

void Pushable::update(automa::ServiceProvider& svc, Map& map, player::Player& player) {

	m_intro.update();
	m_blink.update();
	get_collider().physics.acceleration = {};

	collision_box.set_position(get_collider().physics.position - sf::Vector2f{0.f, 1.f});
	energy = ccm::ext::clamp(energy - dampen, 0.f, std::numeric_limits<float>::max());
	if (energy < 0.2f) { energy = 0.f; }
	if (svc.ticker.every_x_ticks(20)) { random_offset = random::random_vector_float(-energy, energy); }
	weakened.update();
	if (weakened.is_complete()) { hit_count.start(); }

	// reset position if it's far away, and if the player isn't overlapping the start position
	if (hit_count.get_count() > 2 || map.off_the_bottom(get_collider().physics.position)) {
		bool can_respawn = true;
		if (player.get_collider().bounding_box.overlaps(start_box)) { can_respawn = false; }
		for (auto& pushable : map.pushables) {
			if (pushable->get_bounding_box().overlaps(start_box) && pushable.get() != this) { can_respawn = false; }
		}
		if (can_respawn) {
			reset(svc, map);
			svc.soundboard.flags.world.set(audio::World::small_crash);
		}
		if (!has_flag_set(PushableFlags::trying_to_respawn)) { m_blink.reset(); }
		hit_count.start();
		set_flag(PushableFlags::trying_to_respawn, !can_respawn);
	} else {
		set_flag(PushableFlags::trying_to_respawn, false);
	}

	// player pushes block
	set_flag(PushableFlags::pushed, false);
	handle_collision(player.get_collider());
	if (player.get_collider().wallslider.overlaps(collision_box) && player.pushing() && player.is_in_animation(player::AnimState::push) && get_collider().physics.actual_velocity().y < 0.3f) {
		if (player.controller.moving_left() && player.get_collider().physics.position.x > get_collider().physics.position.x) { get_collider().physics.acceleration.x = -speed / mass; }
		if (player.controller.moving_right() && player.get_collider().physics.position.x < get_collider().physics.position.x) { get_collider().physics.acceleration.x = speed / mass; }
		if (ccm::abs(get_collider().physics.actual_velocity().x) > constants::small_value) { svc.soundboard.repeat_sound("pushable_move"); }

		set_flag(PushableFlags::moved);
		set_flag(PushableFlags::pushed);
	}

	for (auto& pushable : map.pushables) {
		if (pushable.get() == this) { continue; }
		if (get_collider().jumpbox.overlaps(pushable->get_collider().predictive_vertical) && !has_flag_set(PushableFlags::pushed)) { get_collider().physics.adopt(pushable->get_collider().physics); }
		if (pushable->get_collider().wallslider.overlaps(collision_box)) {
			if (pushable->get_collider().pushes(*m_collider.get()) && get_collider().grounded()) { set_flag(PushableFlags::pushed); }
		}
	}

	if (size == 1 && get_collider().get_center().y < player.get_collider().get_center().y) { get_collider().handle_collider_collision(player.hurtbox); } // big ones should crush the player
	if (player.get_collider().jumpbox.overlaps(get_collider().bounding_box) && get_collider().grounded() && get_collider().physics.is_moving_horizontally(constants::tiny_value)) {
		player.get_collider().physics.forced_momentum = get_collider().physics.forced_momentum;
	}

	auto test_position = is_moving() ? get_collider().get_center() : get_collider().get_bottom();

	for (auto& pushable : map.pushables) {
		if (pushable.get() == this) { continue; }
		auto block = true;
		if (pushable->get_collider().wallslider.overlaps(collision_box)) {
			auto hit_wall = (pushable->get_collider().get_center().x < get_collider().get_center().x && get_collider().has_right_wallslide_collision()) ||
							(pushable->get_collider().get_center().x > get_collider().get_center().x && get_collider().has_left_wallslide_collision());
			if (pushable->get_collider().pushes(*m_collider.get()) && has_flag_set(PushableFlags::pushed) && !hit_wall) {
				get_collider().physics.acceleration.x += pushable->get_collider().physics.acceleration.x;
				block = false;
			}
		}
		if (pushable->get_collider().jumpbox.overlaps(collision_box)) { block = true; }
		if (get_collider().jumpbox.overlaps(pushable->collision_box)) { block = true; }
		if (block) { pushable->get_collider().handle_collider_collision(collision_box); }
	}

	auto touching_plat = false;
	for (auto& platform : map.platforms) {
		if (get_collider().wallslider.overlaps(platform->get_collider().bounding_box)) { touching_plat = true; }
		if (platform->get_collider().bounding_box.overlaps(get_collider().jumpbox)) { get_collider().handle_collider_collision(platform->get_collider().bounding_box); }
		if (get_collider().jumpbox.overlaps(platform->get_collider().bounding_box) && !get_collider().perma_grounded() && platform->is_sticky() && !touching_plat) {
			get_collider().physics.forced_momentum = platform->get_velocity();
			if (player.get_collider().jumpbox.overlaps(collision_box) && !player.get_collider().perma_grounded() && !(player.get_collider().has_left_wallslide_collision() || player.get_collider().has_right_wallslide_collision())) {
				player.get_collider().physics.forced_momentum = get_collider().physics.forced_momentum;
			}
		}
		if (platform->get_collider().bounding_box.overlaps(get_collider().bounding_box) && !platform->get_collider().bounding_box.overlaps(get_collider().jumpbox)) {
			platform->switch_directions();
			svc.soundboard.flags.world.set(audio::World::hard_hit);
		}
	}

	get_collider().physics.impart_momentum();
}

void Pushable::post_update(automa::ServiceProvider& svc, Map& map, player::Player& player) {

	if (get_collider().has_flag_set(shape::ColliderFlags::landed)) {
		auto point = size == 1 ? get_collider().get_top() : get_collider().get_center();
		if (!m_intro.running()) {
			map.effects.push_back(entity::Effect(svc, "dust", point));
			svc.soundboard.flags.world.set(audio::World::thud);
		}
		get_collider().set_flag(shape::ColliderFlags::landed, false);
	}

	if (!get_collider().has_jump_collision()) { get_collider().physics.forced_momentum = {}; }
	if (get_collider().has_left_wallslide_collision() || get_collider().has_right_wallslide_collision() || get_collider().flags.external_state.test(shape::ExternalState::vert_world_collision) || get_collider().world_grounded()) {
		get_collider().physics.forced_momentum = {};
	}
}

void Pushable::handle_collision(shape::Collider& other) const { other.handle_collider_collision(*m_collider.get()); }

void Pushable::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	Drawable::set_position(collision_box.get_position() - cam + random_offset);
	if (svc.greyblock_mode()) {
		get_collider().render(win, cam);
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
		has_flag_set(PushableFlags::pushed) ? coll.setOutlineColor(colors::mythic_green) : is_moving() ? coll.setOutlineColor(colors::goldenrod) : coll.setOutlineColor(colors::dark_goldenrod);
		coll.setPosition(collision_box.get_position() - cam);
		win.draw(coll);
	} else {
		m_blink.get_normalized() < 0.25f  ? m_return_indicator.setOutlineColor(colors::red)
		: m_blink.get_normalized() < 0.5f ? m_return_indicator.setOutlineColor(colors::dark_fucshia)
										  : m_return_indicator.setOutlineColor(colors::transparent);
		m_blink.get_normalized() < 0.25f ? m_return_indicator.setOutlineThickness(-4.f) : m_blink.get_normalized() < 0.5f ? m_return_indicator.setOutlineThickness(-2.f) : m_return_indicator.setOutlineThickness(-4.f);

		m_return_indicator.setPosition(start_position - cam);
		if (has_flag_set(PushableFlags::trying_to_respawn)) { win.draw(m_return_indicator); }
		win.draw(*this);
	}
}

void Pushable::on_hit(automa::ServiceProvider& svc, world::Map& map, arms::Projectile& proj) {
	if (proj.transcendent()) { return; }
	if (proj.get_collider().collides_with(get_collider().bounding_box)) {
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
	map.effects.push_back(entity::Effect(svc, label, get_collider().get_center()));
	get_collider().physics.position = start_position;
	map.effects.push_back(entity::Effect(svc, label, get_collider().get_center()));
	energy = 0.f;
}

} // namespace fornani::world
