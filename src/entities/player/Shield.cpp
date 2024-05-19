#include "Shield.hpp"
#include "../../service/ServiceProvider.hpp"

namespace player {

Shield::Shield(automa::ServiceProvider& svc) {
	sprite.setTexture(svc.assets.t_shield);
	sensor.bounds.setRadius(28);
	sensor.bounds.setOrigin({28, 28});
	animation.set_params(neutral);
	dimensions = {64, 64};
	health.set_max(16.f);
}

void Shield::start() {
	timer.start(stats.time);
	flags.state.set(ShieldState::shielding);
}

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
	if (flags.state.test(ShieldState::shielding)) { health.inflict(0.01f); }
	if (flags.state.test(ShieldState::popping)) {
		if (flags.triggers.test(ShieldTrigger::shield_down)) {
			svc.soundboard.flags.player.set(audio::Player::shield_drop);
			flags.triggers.reset(ShieldTrigger::shield_down);
		}
		if (animation.complete()) {
			flags = {};
			animation.set_params(neutral);
		}
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
