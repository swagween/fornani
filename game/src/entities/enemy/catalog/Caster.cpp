#include "fornani/entities/enemy/catalog/Caster.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Random.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::enemy {

Caster::Caster(automa::ServiceProvider& svc, world::Map& map)
	: Enemy(svc, "caster"), m_services(&svc), m_map(&map), parts{.scepter{svc.assets.get_texture("caster_scepter"), 2.0f, 0.85f, {-16.f, 38.f}}, .wand{svc.assets.get_texture("caster_wand"), 2.0f, 0.85f, {-40.f, 48.f}}},
	  energy_ball(svc, 3) {
	animation.set_params(dormant);
	collider.physics.maximum_velocity = {8.f, 12.f};
	collider.physics.air_friction = {0.9f, 0.9f};
	collider.flags.general.set(shape::General::complex);
	secondary_collider = shape::Collider({22.f, 22.f});
	directions.desired.lnr = LNR::left;
	directions.actual.lnr = LNR::left;
	directions.movement.lnr = LNR::neutral;
	energy_ball.get().set_team(arms::Team::guardian);

	target = vfx::Gravitator(sf::Vector2f{}, sf::Color::Transparent, 0.013f);
	target.collider.physics = components::PhysicsComponent(sf::Vector2f{0.96f, 0.98f}, 1.0f);
	target.collider.physics.maximum_velocity = sf::Vector2f(20.f, 20.f);

	variant = util::random::percent_chance(15) ? CasterVariant::tyrant : CasterVariant::apprentice;
	if (variant == CasterVariant::apprentice) { flags.general.reset(GeneralFlags::rare_drops); }

	cooldowns.awaken.start();
	parts.scepter.sprite->setTextureRect(sf::IntRect{{0, 0}, scepter_dimensions});
	parts.wand.sprite->setTextureRect(sf::IntRect{{0, 0}, wand_dimensions});
}

void Caster::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	if (died()) {
		Enemy::update(svc, map, player);
		return;
	}
	if (!flags.state.test(StateFlags::invisible)) {
		target.update(svc);
		// set target based on player position
		auto idle_distance = sf::Vector2f{100.f, -160.f};
		auto signal_distance = sf::Vector2f{-300.f, 0.f};
		auto standard_distance = sf::Vector2f{-60.f, -140.f};
		if (state == CasterState::idle) {
			if (player.collider.grounded()) {
			} else {
				idle_distance.y = -120.f;
			}
			if (directions.actual.lnr == LNR::right) { idle_distance.x *= -1; }
			if (util::random::percent_chance(12) && svc.ticker.every_x_ticks(10)) { idle_distance.x *= -1; }
			target.set_target_position(player.collider.get_center() + idle_distance);
		}
		if (state == CasterState::signal) {
			if (player.collider.grounded()) {
				signal_distance.y = 0.f;
			} else {
				signal_distance.y = -80.f;
			}
			if (directions.actual.lnr == LNR::right) { signal_distance.x *= -1; }
			target.set_target_position(player.collider.get_center() + signal_distance);
		} else if (!is_dormant()) {
			if (directions.actual.lnr == LNR::right) { standard_distance.x *= -1; }
			target.set_target_position(player.collider.get_center() + standard_distance);
		} else {
			target.set_position(collider.physics.position);
		}
		if (!is_dormant()) { collider.physics.position = target.position(); }
	}

	energy_ball.update(svc, map, *this);
	auto bp = collider.get_center();
	bp.y -= 48.f;
	energy_ball.get().set_barrel_point(bp);
	attack_target = player.collider.get_center() - energy_ball.barrel_point();

	cooldowns.post_cast.update();
	if (flags.state.test(StateFlags::invisible)) {
		cooldowns.invisibility.update();
		if (cooldowns.invisibility.is_complete()) {
			flags.state.reset(StateFlags::invisible);
			m_map->effects.push_back(entity::Effect(*m_services, "small_flash", collider.physics.position, {}, 0, 4));
		}
	}

	if (flags.state.test(StateFlags::invisible)) {
		flags.state.reset(StateFlags::vulnerable);
	} else {
		flags.state.set(StateFlags::vulnerable);
	}

	// reset animation states to determine next animation state
	state = {};
	directions.desired.lnr = (player.collider.get_center().x < collider.get_center().x) ? LNR::left : LNR::right;
	directions.movement.lnr = target.collider.physics.velocity.x > 0.f ? LNR::right : LNR::left;
	Enemy::update(svc, map, player);
	if (!is_dormant()) {
		parts.scepter.update(svc, map, player, directions.actual, Drawable::get_scale(), collider.get_center());
		parts.wand.update(svc, map, player, directions.actual, Drawable::get_scale(), collider.get_center());
	}

	secondary_collider.physics.position = collider.physics.position - sf::Vector2f{0.f, 10.f};
	secondary_collider.physics.position.x += directions.actual.lnr == LNR::left ? 2.f : collider.dimensions.x - secondary_collider.dimensions.x - 2.f;
	secondary_collider.sync_components();

	if (flags.state.test(StateFlags::hurt) && !sound.hurt_sound_cooldown.running()) {
		m_services->soundboard.flags.demon.set(audio::Demon::hurt);
		sound.hurt_sound_cooldown.start();
		hurt_effect.start(128);
		flags.state.reset(StateFlags::hurt);
		cooldowns.pre_invisibility.start();
	}
	hurt_effect.update();
	if (cooldowns.pre_invisibility.is_almost_complete()) {
		teleport();
		cooldowns.pre_invisibility.cancel();
	}

	if (is_alert() && !is_hostile() && !cooldowns.post_cast.running()) { state = CasterState::prepare; }

	if (just_died()) { m_services->soundboard.flags.demon.set(audio::Demon::death); }

	if (directions.actual.lnr != directions.desired.lnr) { state = CasterState::turn; }

	state_function = state_function();
}

void Caster::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	if (died() || state == CasterState::dormant || flags.state.test(StateFlags::invisible)) { return; }
	variant == CasterVariant::apprentice ? parts.scepter.render(svc, win, cam) : parts.wand.render(svc, win, cam);
	if (svc.greyblock_mode()) {}
}

void Caster::teleport() {
	auto original = collider.physics.position;
	bool done{};
	int ctr{};
	while (!done && ctr < 32) {
		auto attempt = util::random::random_vector_float(-300.f, 300.f);
		collider.physics.position += attempt;
		collider.sync_components();
		if (m_map->overlaps_middleground(collider.bounding_box)) {
			collider.physics.position = original;
			collider.sync_components();
		} else {
			done = true;
			m_map->effects.push_back(entity::Effect(*m_services, "smoke", original, {}, 2, 0));
			m_services->soundboard.flags.world.set(audio::World::block_toggle);
			target.set_position(collider.physics.position);
			parts.wand.set_position(collider.get_center());
			parts.scepter.set_position(collider.get_center());
			flags.state.set(StateFlags::invisible);
			cooldowns.invisibility.start();
		}
		++ctr;
	}
}

fsm::StateFunction Caster::update_idle() {
	animation.label = "idle";
	cooldowns.pre_invisibility.update();
	if (animation.just_started()) { flags.state.reset(StateFlags::hostile); }
	if (change_state(CasterState::turn, turn)) { return CASTER_BIND(update_turn); }
	if (change_state(CasterState::prepare, prepare)) { return CASTER_BIND(update_prepare); }
	state = CasterState::idle;
	return CASTER_BIND(update_idle);
};

fsm::StateFunction Caster::update_turn() {
	animation.label = "turn";
	cooldowns.pre_invisibility.update();
	if (animation.complete()) {
		flip();
		directions.actual = directions.desired;
		state = CasterState::idle;
		animation.set_params(idle);
		return CASTER_BIND(update_idle);
	}
	state = CasterState::turn;
	return CASTER_BIND(update_turn);
}
fsm::StateFunction Caster::update_prepare() {
	cooldowns.pre_invisibility.update();
	if (animation.complete()) {
		state = CasterState::signal;
		animation.set_params(signal);
		return CASTER_BIND(update_signal);
	}
	state = CasterState::prepare;
	return CASTER_BIND(update_prepare);
};

fsm::StateFunction Caster::update_signal() {
	animation.label = "signal";
	if (animation.just_started()) {
		auto sign = directions.actual.lnr == LNR::left ? 1.f : -1.f;
		parts.scepter.sprite->rotate(sf::degrees(90.f) * sign);
		cooldowns.rapid_fire.start(208);
	}
	if (m_services->ticker.every_x_ticks(20)) { flash.update(); }
	parts.scepter.sprite->setTextureRect(sf::IntRect{{0, 20 + 20 * flash.get_alternator()}, scepter_dimensions});
	parts.wand.sprite->setTextureRect(sf::IntRect{{0, 62 + 62 * flash.get_alternator()}, wand_dimensions});
	if (variant == CasterVariant::tyrant) { cooldowns.rapid_fire.update(); }
	if (cooldowns.rapid_fire.is_almost_complete()) {
		m_map->spawn_projectile_at(*m_services, energy_ball.get(), energy_ball.get().get_barrel_point(), attack_target);
		cooldowns.rapid_fire.start();
		m_services->soundboard.flags.weapon.set(audio::Weapon::energy_ball);
	}
	if (animation.complete()) {
		parts.scepter.sprite->setTextureRect(sf::IntRect{{0, 0}, scepter_dimensions});
		parts.wand.sprite->setTextureRect(sf::IntRect{{0, 0}, wand_dimensions});
		auto sign = directions.actual.lnr == LNR::left ? 1.f : -1.f;
		parts.scepter.sprite->rotate(sf::degrees(-90.f) * sign);
		cooldowns.post_cast.start();
		if (variant == CasterVariant::apprentice) {
			m_map->spawn_projectile_at(*m_services, energy_ball.get(), energy_ball.get().get_barrel_point(), attack_target);
			m_services->soundboard.flags.weapon.set(audio::Weapon::energy_ball);
		} // only shoot at end for apprentice
		if (directions.actual.lnr != directions.desired.lnr) {
			state = CasterState::turn;
			animation.set_params(turn);
			return CASTER_BIND(update_turn);
		}
		state = CasterState::idle;
		animation.set_params(idle);
		return CASTER_BIND(update_idle);
	}
	state = CasterState::signal;
	return CASTER_BIND(update_signal);
}

fsm::StateFunction Caster::update_dormant() {
	flags.state.reset(StateFlags::vulnerable);
	flags.general.reset(GeneralFlags::foreground);
	is_hostile() ? cooldowns.awaken.update() : cooldowns.awaken.reverse();
	if (cooldowns.awaken.halfway()) {
		shake();
		m_services->soundboard.flags.world.set(audio::World::pushable_move);
	}
	if (cooldowns.awaken.is_complete() || flags.state.test(StateFlags::shot)) {
		cooldowns.awaken.cancel();
		flags.state.set(StateFlags::vulnerable);
		m_map->effects.push_back(entity::Effect(*m_services, "small_explosion", collider.physics.position, {}, 2, 0));
		m_services->soundboard.flags.world.set(audio::World::block_toggle);
		state = CasterState::idle;
		animation.set_params(idle);
		flags.general.set(GeneralFlags::foreground);
		return CASTER_BIND(update_idle);
	}
	state = CasterState::dormant;
	return CASTER_BIND(update_dormant);
}

bool Caster::change_state(CasterState next, anim::Parameters params) {
	if (state == next) {
		animation.set_params(params, false);
		return true;
	}
	return false;
}

} // namespace fornani::enemy
