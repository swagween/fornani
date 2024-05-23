#include "Minigus.hpp"
#include "../../../level/Map.hpp"
#include "../../../service/ServiceProvider.hpp"
#include "../../player/Player.hpp"

namespace enemy {

Minigus::Minigus(automa::ServiceProvider& svc) : Enemy(svc, "minigus"), gun(svc, "minigun", 6) , m_services(&svc), npc::NPC(svc, 7) {
	animation.set_params(idle);
	gun.clip_cooldown_time = 360;
	gun.get().projectile.team = arms::TEAMS::SKYCORPS;
	Enemy::collider.physics.maximum_velocity = {3.f, 12.f};
	Enemy::collider.physics.air_friction = {0.95f, 0.999f};
	minigun.sprite.setTexture(svc.assets.t_minigun);
	minigun.animation.set_params(minigun.neutral);
}

void Minigus::unique_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {

	flags.state.set(StateFlags::vulnerable); // minigus is always vulnerable
	gun.update(svc, map, *this);
	caution.avoid_ledges(map, Enemy::collider, 1);
	running_time.update();
	minigun.animation.update();

	state = MinigusState::idle;
	if (state == MinigusState::shoot) {
		if (!gun.get().cooling_down()) {
			gun.cycle.update();
			gun.barrel_offset = gun.cycle.get_alternator() % 2 == 0 ? sf::Vector2<float>{-14.f, 10.f} : sf::Vector2<float>{26.f, 20.f};
			gun.shoot();
			map.spawn_projectile_at(svc, gun.get(), gun.barrel_point());
			m_services->soundboard.flags.weapon.set(audio::Weapon::skycorps_ar);
		}
	}

	Enemy::direction.lr = (player.collider.physics.position.x < Enemy::collider.physics.position.x) ? dir::LR::left : dir::LR::right;
	Enemy::update(svc, map, player);

	if (hostility_triggered() && gun.clip_cooldown.is_complete() && running_time.is_complete()) {
	}
	if (hostile() && !hostility_triggered() && gun.clip_cooldown.is_complete()) {
		if (m_services->random.percent_chance(fire_chance) || caution.danger(Enemy::direction)) {
			state = MinigusState::shoot;
		}
	} // player is already in hostile range

	if (caution.danger(Enemy::direction)) { running_time.cancel(); }
	if (running_time.is_complete() && gun.clip_cooldown.is_complete()) {
		state = MinigusState::idle;
	} else if (!running_time.is_complete()) {
		state = MinigusState::run;
	}

	if (svc.ticker.every_x_ticks(200)) {
		if (svc.random.percent_chance(10) && !caution.danger(Enemy::direction) && !running_time.running()) {
			state = MinigusState::run;
			running_time.start(300);
		}
	}

	if (running_time.is_complete()) { state = MinigusState::idle; }

	if (just_died()) {  }

	if (Enemy::ent_state.test(entity::State::flip)) { state = MinigusState::turn; }

	state_function = state_function();
}

void Minigus::unique_render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	auto u = minigun.animation.get_frame() > 13 ? 78 : 0;
	auto v = minigun.animation.get_frame() * 30;
	minigun.sprite.setTextureRect(sf::IntRect({{u, v}, sf::Vector2<int>{78, 30}}));
	minigun.sprite.setPosition(Enemy::sprite.getPosition() + minigun.offset - Enemy::sprite.getOrigin());
	if (!svc.greyblock_mode()) { win.draw(minigun.sprite); }
}

fsm::StateFunction Minigus::update_idle() {
	if (change_state(MinigusState::turn, turn)) { return MINIGUS_BIND(update_turn); }
	if (change_state(MinigusState::jumpsquat, jumpsquat)) { return MINIGUS_BIND(update_jumpsquat); }
	if (change_state(MinigusState::shoot, shoot)) { return MINIGUS_BIND(update_shoot); }
	if (change_state(MinigusState::run, run)) { return MINIGUS_BIND(update_run); }
	if (change_state(MinigusState::hurt, hurt)) { return MINIGUS_BIND(update_hurt); }
	if (change_state(MinigusState::reload, reload)) { return MINIGUS_BIND(update_reload); }
	state = MinigusState::idle;
	return MINIGUS_BIND(update_idle);
}

fsm::StateFunction Minigus::update_shoot() {
	if (animation.complete()) {
		if (change_state(MinigusState::idle, idle)) { return MINIGUS_BIND(update_turn); }
		if (change_state(MinigusState::jumpsquat, jumpsquat)) { return MINIGUS_BIND(update_jumpsquat); }
		if (change_state(MinigusState::run, run)) { return MINIGUS_BIND(update_run); }
		if (change_state(MinigusState::hurt, hurt)) { return MINIGUS_BIND(update_hurt); }
		if (change_state(MinigusState::reload, reload)) { return MINIGUS_BIND(update_reload); }
	}
	state = MinigusState::run;
	return MINIGUS_BIND(update_shoot);
}

fsm::StateFunction Minigus::update_jumpsquat() { return MINIGUS_BIND(update_idle); }

fsm::StateFunction Minigus::update_hurt() {
	if (animation.complete()) {
		if (change_state(MinigusState::idle, idle)) { return MINIGUS_BIND(update_turn); }
		if (change_state(MinigusState::jumpsquat, jumpsquat)) { return MINIGUS_BIND(update_jumpsquat); }
		if (change_state(MinigusState::shoot, shoot)) { return MINIGUS_BIND(update_shoot); }
		if (change_state(MinigusState::run, run)) { return MINIGUS_BIND(update_run); }
		if (change_state(MinigusState::reload, reload)) { return MINIGUS_BIND(update_reload); }
	}
	state = MinigusState::hurt;
	return MINIGUS_BIND(update_hurt);
}

fsm::StateFunction Minigus::update_jump() { return MINIGUS_BIND(update_idle); }

fsm::StateFunction Minigus::update_jump_shoot() { return MINIGUS_BIND(update_idle); }

fsm::StateFunction Minigus::update_reload() { return MINIGUS_BIND(update_idle); }

fsm::StateFunction Minigus::update_turn() {
	if (animation.complete()) {
		Enemy::sprite_flip();
		if (change_state(MinigusState::idle, idle)) { return MINIGUS_BIND(update_idle); }
		if (change_state(MinigusState::jumpsquat, jumpsquat)) { return MINIGUS_BIND(update_jumpsquat); }
		if (change_state(MinigusState::shoot, shoot)) { return MINIGUS_BIND(update_shoot); }
		if (change_state(MinigusState::run, run)) { return MINIGUS_BIND(update_run); }
		if (change_state(MinigusState::hurt, hurt)) { return MINIGUS_BIND(update_hurt); }
		if (change_state(MinigusState::reload, reload)) { return MINIGUS_BIND(update_reload); }
		state = MinigusState::idle;
		animation.set_params(idle);
		return MINIGUS_BIND(update_idle);
	}
	state = MinigusState::turn;
	return MINIGUS_BIND(update_turn);
}

fsm::StateFunction Minigus::update_run() {
	auto sign = Enemy::direction.lr == dir::LR::left ? -1 : 1;
	Enemy::collider.physics.apply_force({Enemy::attributes.speed * sign, 0.f});
	if (change_state(MinigusState::idle, idle)) { return MINIGUS_BIND(update_turn); }
	if (change_state(MinigusState::jumpsquat, jumpsquat)) { return MINIGUS_BIND(update_jumpsquat); }
	if (change_state(MinigusState::shoot, shoot)) { return MINIGUS_BIND(update_shoot); }
	if (change_state(MinigusState::hurt, hurt)) { return MINIGUS_BIND(update_hurt); }
	if (change_state(MinigusState::reload, reload)) { return MINIGUS_BIND(update_reload); }
	if (change_state(MinigusState::turn, turn)) { return MINIGUS_BIND(update_turn); }
	state = MinigusState::run;
	return MINIGUS_BIND(update_run);
}

bool Minigus::change_state(MinigusState next, anim::Parameters params) {
	if (state == next) {
		animation.set_params(params);
		return true;
	}
	return false;
}

} // namespace enemy