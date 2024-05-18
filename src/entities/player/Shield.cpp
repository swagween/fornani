#include "Shield.hpp"
#include "../../service/ServiceProvider.hpp"

namespace player {

Shield::Shield(automa::ServiceProvider& svc) { sprite.setTexture(svc.assets.t_shield);}

void Shield::start() {
	timer.start(stats.time);
	flags.state.set(ShieldState::shielding);
	sensor.bounds.setRadius(28);
}

void Shield::end() { flags.state.reset(ShieldState::shielding); }

void Shield::update() {
	timer.update();
	animation.update();
}

void Shield::reset_triggers() { flags.triggers = {}; }

void Shield::reset_all() {
	flags.triggers = {};
	flags.state = {};
}

void Shield::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	sf::Vector2<float> offset{4.f, 4.f};
	auto state = flags.state.test(ShieldState::shielding) ? 0 : flags.state.test(ShieldState::wobbling) ? 1 : 2;
	int u = state * svc.constants.cell_size;
	int v = animation.get_frame() * svc.constants.cell_size;
	sprite.setTextureRect(sf::IntRect{{u, v}, {64, 64}});
	sprite.setPosition(sensor.bounds.getPosition() - offset);
	if (svc.greyblock_mode()) {
		win.draw(sensor.bounds);
	} else {
		win.draw(sprite);
	}
}

} // namespace player
