#include "fornani/entities/enemy/catalog/Archer.hpp"
#include "fornani/level/Map.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/entities/player/Player.hpp"

namespace enemy {

Archer::Archer(automa::ServiceProvider& svc, world::Map& map)
	: Enemy(svc, "archer"), m_services(&svc), m_map(&map), parts{.bow{svc.assets.t_archer_bow, 0.8f, 0.85f, {-42.f, -34.f}}} {
	animation.set_params(idle);
	collider.physics.maximum_velocity = {8.f, 12.f};
	collider.physics.air_friction = {0.95f, 0.999f};
	collider.flags.general.set(shape::General::complex);
	directions.desired.lr = dir::LR::left;
	directions.actual.lr = dir::LR::left;
	directions.movement.lr = dir::LR::neutral;

	variant = svc.random.percent_chance(70) ? ArcherVariant::huntress : ArcherVariant::defender;
	parts.bow.sprite.setTextureRect(sf::IntRect{{0, 0}, bow_dimensions});
	parts.bow.sprite.setOrigin({32.f, 32.f});

}

void Archer::unique_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	if (died()) {
		Enemy::update(svc, map, player);
		return;
	}

	cooldowns.post_jump.update();

	flags.state.set(StateFlags::vulnerable); // archer is always vulnerable
	caution.avoid_ledges(map, collider, directions.actual, 3);

	// reset animation states to determine next animation state
	directions.desired.lr = (player.collider.get_center().x < collider.get_center().x) ? dir::LR::left : dir::LR::right;
	directions.movement.lr = collider.physics.velocity.x > 0.f ? dir::LR::right : dir::LR::left;
	if (directions.actual.lr == dir::LR::right && visual.sprite.getScale() == sf::Vector2<float>{1.f, 1.f}) { visual.sprite.scale({-1.f, 1.f}); }
	if (directions.actual.lr == dir::LR::left && visual.sprite.getScale() == sf::Vector2<float>{-1.f, 1.f}) { visual.sprite.scale({-1.f, 1.f}); }
	Enemy::update(svc, map, player);
	auto shooting_offset = state == ArcherState::shoot ? sf::Vector2<float>{0.f, -6.f} : sf::Vector2<float>{0.f, 0.f};
	parts.bow.update(svc, map, player, directions.actual, visual.sprite.getScale(), collider.get_center() + shooting_offset);

	if (svc.ticker.every_x_ticks(200)) {
		if (svc.random.percent_chance(4) && !caution.danger()) { state = ArcherState::run; }
	}

	if(flags.state.test(StateFlags::hurt) && !sound.hurt_sound_cooldown.running()) {
		m_services->soundboard.flags.archer.set(audio::Archer::hurt);
		sound.hurt_sound_cooldown.start();
		hurt_effect.start(128);
		flags.state.reset(StateFlags::hurt);
	}

	hurt_effect.update();
	if (hurt_effect.running()) {
		if ((hurt_effect.get_cooldown() / 32) % 2 == 0) {
			visual.sprite.setColor(svc.styles.colors.white);
		} else {
			visual.sprite.setColor(svc.styles.colors.goldenrod);
		}
	} else {
		visual.sprite.setColor(svc.styles.colors.white);
	}
	if (hostile() && !hostility_triggered() && !cooldowns.post_jump.running()) {
		state = ArcherState::run;
	}

	if (alert() && !hostile() && svc.ticker.every_x_ticks(900)) { state = ArcherState::shoot; }

	if (just_died()) { m_services->soundboard.flags.archer.set(audio::Archer::death); }

	if (directions.actual.lr != directions.desired.lr) { state = ArcherState::turn; }

	state_function = state_function();
}

void Archer::unique_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	if (died()) { return; }
	parts.bow.render(svc, win, cam);
	if (svc.greyblock_mode()) {}
}

fsm::StateFunction Archer::update_idle() {
	animation.label = "idle";
	if (animation.just_started()) { flags.state.reset(StateFlags::hostile); }
	if (change_state(ArcherState::turn, turn)) { return ARCHER_BIND(update_turn); }
	if (change_state(ArcherState::run, run)) { return ARCHER_BIND(update_run); }
	if (change_state(ArcherState::shoot, shoot)) { return ARCHER_BIND(update_shoot); }
	state = ArcherState::idle;
	return ARCHER_BIND(update_idle);
};

fsm::StateFunction Archer::update_turn() {
	animation.label = "turn";
	if (animation.complete()) {
		visual.sprite.scale({-1.f, 1.f});
		directions.actual = directions.desired;
		state = ArcherState::idle;
		animation.set_params(idle, false);
		return ARCHER_BIND(update_idle);
	}
	state = ArcherState::turn;
	return ARCHER_BIND(update_turn);
};

fsm::StateFunction Archer::update_run() {
	animation.label = "run";
	auto facing = directions.actual.lr == dir::LR::left ? -1.f : 1.f;
	collider.physics.apply_force({attributes.speed * facing, 0.f});
	if (caution.danger() || animation.complete()) {
		state = ArcherState::idle;
		animation.set_params(idle);
		return ARCHER_BIND(update_idle);
	}
	if (change_state(ArcherState::turn, turn)) { return ARCHER_BIND(update_turn); }
	state = ArcherState::run;
	return ARCHER_BIND(update_run);
}

fsm::StateFunction Archer::update_jump() {
	animation.label = "jump";
	if (animation.just_started()) {
		cooldowns.jump.start();
		rand_jump = m_services->random.percent_chance(50) ? -1.f : 1.f;
	}
	if (cooldowns.jump.running()) { collider.physics.apply_force({0, -2.5f}); }
	if (!collider.grounded()) { collider.physics.apply_force({rand_jump * 2.f, 0.f}); }
	cooldowns.jump.update();
	if (cooldowns.jump.is_complete() && collider.grounded()) {
		cooldowns.post_jump.start();
		state = ArcherState::idle;
		animation.set_params(idle);
		return ARCHER_BIND(update_idle);
	}
	state = ArcherState::jump;
	return ARCHER_BIND(update_jump);
}

fsm::StateFunction Archer::update_shoot() {
	auto bow_frame = animation.frame_timer.get_cooldown() >= animation.params.framerate / 2 ? 1 : 2;
	parts.bow.sprite.setTextureRect(sf::IntRect{{82 * bow_frame, 0}, bow_dimensions});
	if(animation.complete()) {
		state = ArcherState::idle;
		animation.set_params(idle);
		parts.bow.sprite.setTextureRect(sf::IntRect{{0, 0}, bow_dimensions});
		return ARCHER_BIND(update_idle);
	}
	state = ArcherState::shoot;
	return ARCHER_BIND(update_shoot);
}

bool Archer::change_state(ArcherState next, anim::Parameters params) {
	if (state == next) {
		animation.set_params(params, true);
		return true;
	}
	return false;
}

} // namespace enemy