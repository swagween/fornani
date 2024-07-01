#include "Fader.hpp"
#include "../service/ServiceProvider.hpp"


namespace util {

Fader::Fader(automa::ServiceProvider& svc, int time, std::string_view color) : time(time) {
	sprite.setTexture(svc.assets.t_fader);
	if (color_codes.contains(color)) {
		lookup = color_codes.at(color)[0];
		order = color_codes.at(color)[1];
	}
	timer.start(time);
	interval = static_cast<int>(time / 2);
}

void Fader::update() {
	timer.update();
	if (interval == 0) { interval = 1; }
	if (timer.get_cooldown() % interval == 0) {
		progress.update();
		interval = static_cast<int>(timer.get_cooldown() / 2);
	}
	for (auto i{0}; i < order; ++i) { sprite.setTextureRect(sf::IntRect{{lookup, progress.get_count()}, {1, 1}}); }
}

} // namespace util
