#include "fornani/entities/player/Shield.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::player {

Shield::Shield(automa::ServiceProvider& svc) : sprite{svc.assets.t_shield} {
	sensor.bounds.setRadius(28);
	sensor.bounds.setOrigin({28, 28});
	animation.set_params(neutral);
	dimensions = {64, 64};
	health.set_max(16.0f);
	hud_animation.set_params(hud_animations.not_shielding);
}

void Shield::start() { timer.start(stats.time); }

void Shield::pop() {
	flags.state.set(ShieldState::popping);
	flags.triggers.set(ShieldTrigger::shield_down);
	animation.end();
	animation.set_params(popping);
}

void Shield::end() {}

void Shield::update(automa::ServiceProvider& svc) {
	timer.update();
	animation.update();
	hud_animation.update();
	health.update();
	if (flags.state.test(ShieldState::popping)) {
		if (animation.complete()) {
			flags.state.reset(ShieldState::popping);
			animation.set_params(neutral);
		}
	}

	// recovery mode
	if (flags.state.test(ShieldState::recovery)) {
		if (health.full()) {
			hud_animation.set_params(hud_animations.not_shielding);
			flags.state.reset(ShieldState::recovery);
		}
		health.inflict(-stats.recovery_regen_rate);
		return;
	}
	// exit early if shield is recovering

	is_shielding() ? health.inflict(stats.depletion_rate) : health.inflict(-stats.regen_rate);
	health.update();
	if (health.is_dead()) {
		pop();
		flags.state.set(ShieldState::recovery);
	}

	if (flags.triggers.test(ShieldTrigger::shield_up)) {
		flags.state.set(ShieldState::shielding);
		hud_animation.set_params(hud_animations.shielding);
		switch_point = health.get_hp();
		flags.triggers.reset(ShieldTrigger::shield_up);
	}
	if (flags.triggers.test(ShieldTrigger::shield_down)) {
		svc.soundboard.flags.player.set(audio::Player::shield_drop);
		hud_animation.set_params(hud_animations.not_shielding);
		switch_point = health.get_hp();
		flags.state.reset(ShieldState::shielding);
		flags.triggers.reset(ShieldTrigger::shield_down);
	}
	if (flags.state.test(ShieldState::recovery)) {
		hud_animation.set_params(hud_animations.recovering);
	}
}

void Shield::damage(float amount) { health.inflict(amount); }

void Shield::reset_triggers() { flags.triggers = {}; }

void Shield::reset_all() {
	flags.triggers = {};
	flags.state = {};
}

void Shield::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	sf::Vector2<float> offset{32.f, 36.f};
	int u = animation.params.lookup * dimensions.x;
	int v = animation.get_frame() * dimensions.y;
	sprite.setTextureRect(sf::IntRect{{u, v}, dimensions});
	sprite.setPosition(sensor.bounds.getPosition() - offset - cam);
	if (svc.greyblock_mode()) {
		win.draw(sensor.bounds);
	} else {
		win.draw(sprite);
	}
}

} // namespace player
