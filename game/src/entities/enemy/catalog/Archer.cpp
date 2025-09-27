#include "fornani/entities/enemy/catalog/Archer.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Random.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::enemy {

Archer::Archer(automa::ServiceProvider& svc, world::Map& map) : Enemy(svc, "archer"), m_services(&svc), m_map(&map), m_bow(svc, "demon_bow"), parts{.bow{svc.assets.get_texture("archer_bow"), 0.8f, 0.85f, {-18.f, -8.f}}} {
	auto archer_framerate = 12;
	m_params = {{"idle", {0, 8, archer_framerate * 2, -1}}, {"turn", {8, 1, archer_framerate * 2, 0}}, {"run", {9, 4, archer_framerate * 2, 4}}, {"jump", {9, 1, archer_framerate * 3, 0}}, {"shoot", {13, 1, archer_framerate * 12, 0}}};
	collider.physics.maximum_velocity = {8.f, 12.f};
	collider.physics.air_friction = {0.95f, 0.999f};
	collider.flags.general.set(shape::General::complex);
	directions.desired.lnr = LNR::left;
	directions.actual.lnr = LNR::left;
	directions.movement.lnr = LNR::neutral;

	variant = random::percent_chance(70) ? ArcherVariant::huntress : ArcherVariant::defender;
	parts.bow.sprite->setTextureRect(sf::IntRect{{0, 0}, bow_dimensions});
	parts.bow.sprite->setOrigin({32.f, 32.f});

	m_bow.get().set_team(arms::Team::guardian);
}

void Archer::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	if (died()) {
		Enemy::update(svc, map, player);
		return;
	}

	auto distance = (player.collider.get_center() - collider.get_center()).length();
	auto correction = sf::Vector2f{0.f, sqrt(distance)};
	m_player_target = player.collider.get_center() - correction;

	cooldowns.post_jump.update();

	flags.state.set(StateFlags::vulnerable); // archer is always vulnerable
	caution.avoid_ledges(map, collider, directions.actual, 3);

	m_bow.update(svc, map, *this);

	// reset animation states to determine next animation state
	directions.desired.lnr = (player.collider.get_center().x < collider.get_center().x) ? LNR::left : LNR::right;
	directions.movement.lnr = collider.physics.velocity.x > 0.f ? LNR::right : LNR::left;
	Enemy::update(svc, map, player);
	auto shooting_offset = is_state(ArcherState::shoot) ? sf::Vector2{0.f, -6.f} : sf::Vector2{0.f, 0.f};
	parts.bow.update(svc, map, player, directions.actual, Drawable::get_scale(), collider.get_center() + shooting_offset);

	if (svc.ticker.every_x_ticks(200)) {
		if (random::percent_chance(4) && !caution.danger()) { request(ArcherState::run); }
	}

	if (flags.state.test(StateFlags::hurt) && !sound.hurt_sound_cooldown.running()) {
		m_services->soundboard.flags.archer.set(audio::Archer::hurt);
		sound.hurt_sound_cooldown.start();
		hurt_effect.start(128);
		flags.state.reset(StateFlags::hurt);
	}

	hurt_effect.update();

	if (is_hostile() && !hostility_triggered() && !cooldowns.post_jump.running()) { request(ArcherState::run); }

	if (is_alert() && !is_hostile() && svc.ticker.every_x_ticks(900)) { request(ArcherState::shoot); }

	if (just_died()) { m_services->soundboard.flags.archer.set(audio::Archer::death); }

	if (directions.actual.lnr != directions.desired.lnr) { request(ArcherState::turn); }

	state_function = state_function();
}

void Archer::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	if (died()) { return; }
	Enemy::render(svc, win, cam);
	parts.bow.render(svc, win, cam);
	if (svc.greyblock_mode()) {}
}

fsm::StateFunction Archer::update_idle() {
	m_state.actual = ArcherState::idle;
	if (animation.just_started()) { flags.state.reset(StateFlags::hostile); }
	if (change_state(ArcherState::turn, get_params("turn"))) { return ARCHER_BIND(update_turn); }
	if (change_state(ArcherState::run, get_params("run"))) { return ARCHER_BIND(update_run); }
	if (change_state(ArcherState::shoot, get_params("shoot"))) { return ARCHER_BIND(update_shoot); }
	return ARCHER_BIND(update_idle);
};

fsm::StateFunction Archer::update_turn() {
	m_state.actual = ArcherState::turn;
	if (animation.complete()) {
		request_flip();
		request(ArcherState::idle);
		if (change_state(ArcherState::idle, get_params("idle"))) { return ARCHER_BIND(update_idle); }
	}
	return ARCHER_BIND(update_turn);
};

fsm::StateFunction Archer::update_run() {
	m_state.actual = ArcherState::run;
	auto const facing = directions.actual.lnr == LNR::left ? -1.f : 1.f;
	collider.physics.apply_force({attributes.speed * facing, 0.f});
	if (caution.danger() || animation.complete()) {
		request(ArcherState::shoot);
		if (change_state(ArcherState::shoot, get_params("shoot"))) { return ARCHER_BIND(update_shoot); }
	}
	if (change_state(ArcherState::turn, get_params("turn"))) { return ARCHER_BIND(update_turn); }
	return ARCHER_BIND(update_run);
}

fsm::StateFunction Archer::update_jump() {
	m_state.actual = ArcherState::jump;
	if (animation.just_started()) {
		cooldowns.jump.start();
		rand_jump = random::percent_chance(50) ? -1.f : 1.f;
	}
	if (cooldowns.jump.running()) { collider.physics.apply_force({0, -2.5f}); }
	if (!collider.grounded()) { collider.physics.apply_force({rand_jump * 2.f, 0.f}); }
	cooldowns.jump.update();
	if (cooldowns.jump.is_complete() && collider.grounded()) {
		cooldowns.post_jump.start();
		request(ArcherState::idle);
		if (change_state(ArcherState::idle, get_params("idle"))) { return ARCHER_BIND(update_idle); }
	}
	return ARCHER_BIND(update_jump);
}

fsm::StateFunction Archer::update_shoot() {
	m_state.actual = ArcherState::shoot;
	auto bow_frame = animation.frame_timer.get() >= animation.params.framerate / 2 ? 1 : 2;
	parts.bow.sprite->setTextureRect(sf::IntRect{{41 * bow_frame, 0}, bow_dimensions});
	if (animation.complete()) {
		auto bp = collider.get_center();
		bp.x += 24.f * directions.actual.as_float();
		bp.y -= 48.f;
		m_bow.get().set_barrel_point(bp);
		m_map->spawn_projectile_at(*m_services, m_bow.get(), m_bow.get().get_barrel_point(), m_player_target - m_bow.get().get_barrel_point());
		parts.bow.sprite->setTextureRect(sf::IntRect{{0, 0}, bow_dimensions});
		request(ArcherState::idle);
		if (change_state(ArcherState::idle, get_params("idle"))) { return ARCHER_BIND(update_idle); }
		return ARCHER_BIND(update_idle);
	}
	return ARCHER_BIND(update_shoot);
}

bool Archer::change_state(ArcherState next, anim::Parameters params) {
	if (m_state.desired == next) {
		animation.set_params(params, true);
		return true;
	}
	return false;
}

} // namespace fornani::enemy
