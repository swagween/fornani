#include "Caster.hpp"
#include "../../../level/Map.hpp"
#include "../../../service/ServiceProvider.hpp"
#include "../../player/Player.hpp"

namespace enemy {

Caster::Caster(automa::ServiceProvider& svc, world::Map& map)
	: Enemy(svc, "caster"), m_services(&svc), m_map(&map),
	  parts{.scepter{svc.assets.t_caster_scepter, 2.0f, 0.85f, {-16.f, 8.f}}, .wand{svc.assets.t_caster_wand, 2.0f, 0.85f, {-4.f, 8.f}}} {
	animation.set_params(dormant);
	collider.physics.maximum_velocity = {8.f, 12.f};
	collider.physics.air_friction = {0.95f, 0.999f};
	collider.flags.general.set(shape::General::complex);
	secondary_collider = shape::Collider({22.f, 22.f});
	directions.desired.lr = dir::LR::left;
	directions.actual.lr = dir::LR::left;
	directions.movement.lr = dir::LR::neutral;

	target = vfx::Gravitator(sf::Vector2<float>{}, sf::Color::Transparent, 0.013f);
	target.collider.physics = components::PhysicsComponent(sf::Vector2<float>{0.96f, 0.98f}, 1.0f);
	target.collider.physics.maximum_velocity = sf::Vector2<float>(20.f, 20.f);

	variant = svc.random.percent_chance(10) ? CasterVariant::tyrant : CasterVariant::apprentice;

	cooldowns.awaken.start();
}

void Caster::unique_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	if (died()) {
		Enemy::update(svc, map, player);
		return;
	}
	target.update(svc);
	// set target based on player position
	auto idle_distance = sf::Vector2<float>{100.f, -160.f};
	auto signal_distance = sf::Vector2<float>{-60.f, 0.f};
	auto standard_distance = sf::Vector2<float>{-60.f, -140.f};
	if (state == CasterState::idle) {
		if (player.collider.grounded()) {
		} else {
			idle_distance.y = -120.f;
		}
		if (directions.actual.lr == dir::LR::right) { idle_distance.x *= -1; }
		if (svc.random.percent_chance(12) && svc.ticker.every_x_ticks(10)) { idle_distance.x *= -1; }
		target.set_target_position(player.collider.get_center() + idle_distance);
	}
	if (state == CasterState::signal) {
		if (player.collider.grounded()) {
			signal_distance.y = 0.f;
		} else {
			signal_distance.y = -80.f;
		}
		if (directions.actual.lr == dir::LR::right) { signal_distance.x *= -1; }
		target.set_target_position(player.collider.get_center() + signal_distance);
	} else if (!is_dormant()) {
		if (directions.actual.lr == dir::LR::right) { standard_distance.x *= -1; }
		target.set_target_position(player.collider.get_center() + standard_distance);
	} else {
		target.set_position(collider.physics.position);
	}
	if (!is_dormant()) { collider.physics.position = target.position(); }


	cooldowns.post_cast.update();

	flags.state.set(StateFlags::vulnerable); // caster is always vulnerable

	// reset animation states to determine next animation state
	state = {};
	directions.desired.lr = (player.collider.get_center().x < collider.get_center().x) ? dir::LR::left : dir::LR::right;
	directions.movement.lr = target.collider.physics.velocity.x > 0.f ? dir::LR::right : dir::LR::left;
	Enemy::update(svc, map, player);
	if (!is_dormant()) {
		parts.scepter.update(svc, map, player, directions.actual, sprite.getScale(), collider.get_center());
		parts.wand.update(svc, map, player, directions.actual, sprite.getScale(), collider.get_center());
	}

	secondary_collider.physics.position = collider.physics.position - sf::Vector2<float>{0.f, 10.f};
	secondary_collider.physics.position.x += directions.actual.lr == dir::LR::left ? 2.f : collider.dimensions.x - secondary_collider.dimensions.x - 2.f;
	secondary_collider.sync_components();

	if(flags.state.test(StateFlags::hurt)) {
		m_services->soundboard.flags.demon.set(audio::Demon::hurt);
		hurt_effect.start();
		flags.state.reset(StateFlags::hurt);
	}
	hurt_effect.update();

	if (hurt_effect.running()) {
		if ((hurt_effect.get_cooldown() / 32) % 2 == 0) {
			sprite.setColor(svc.styles.colors.green);
		} else {
			sprite.setColor(svc.styles.colors.mythic_green);
		}
	} else {
		sprite.setColor(svc.styles.colors.white);
	}

	if (alert() && !hostile() && !cooldowns.post_cast.running()) { state = CasterState::signal; }

	if (hostile()) { /* teleport(); */
	}

	if (just_died()) { m_services->soundboard.flags.demon.set(audio::Demon::death); }

	if (directions.actual.lr != directions.desired.lr) { state = CasterState::turn; }

	state_function = state_function();

	if (directions.actual.lr == dir::LR::right && sprite.getScale() == sf::Vector2<float>{1.f, 1.f}) { sprite.scale({-1.f, 1.f}); }
	if (directions.actual.lr == dir::LR::left && sprite.getScale() == sf::Vector2<float>{-1.f, 1.f}) { sprite.scale({-1.f, 1.f}); }
}

void Caster::unique_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	if (died() || state == CasterState::dormant) { return; }
	variant == CasterVariant::apprentice ? parts.scepter.render(svc, win, cam) : parts.wand.render(svc, win, cam);
	if (svc.greyblock_mode()) {}
}

fsm::StateFunction Caster::update_idle() {
	animation.label = "idle";
	if (animation.just_started()) { flags.state.reset(StateFlags::hostile); }
	if (change_state(CasterState::turn, turn)) { return CASTER_BIND(update_turn); }
	if (change_state(CasterState::signal, signal)) { return CASTER_BIND(update_signal); }
	state = CasterState::idle;
	return CASTER_BIND(update_idle);
};

fsm::StateFunction Caster::update_turn() {
	animation.label = "turn";
	if (animation.complete()) {
		Enemy::sprite.scale({-1.f, 1.f});
		directions.actual = directions.desired;
		state = CasterState::idle;
		animation.set_params(idle);
		return CASTER_BIND(update_idle);
	}
	state = CasterState::turn;
	return CASTER_BIND(update_turn);
};

fsm::StateFunction Caster::update_signal() { 
	animation.label = "signal";
	if (animation.just_started()) {}
	if (animation.complete()) {
		cooldowns.post_cast.start();
		if (directions.actual.lr != directions.desired.lr) {
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
	hostile() ? cooldowns.awaken.update() : cooldowns.awaken.reverse();
	if (cooldowns.awaken.halfway()) {
		shake();
		m_services->soundboard.flags.world.set(audio::World::pushable);
	}
	if (cooldowns.awaken.is_complete() || flags.state.test(StateFlags::shot)) {
		cooldowns.awaken.cancel();
		flags.state.set(StateFlags::vulnerable);
		m_map->effects.push_back(entity::Effect(*m_services, collider.physics.position, {}, 2, 0));
		m_services->soundboard.flags.world.set(audio::World::block_toggle);
		state = CasterState::idle;
		animation.set_params(idle);
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

} // namespace enemy