#include "fornani/utils/Fader.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::util {

Fader::Fader(automa::ServiceProvider& svc, int time, std::string_view color) : time(time), sprite{svc.assets.get_texture("fader")} {
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
	if (timer.get() % interval == 0) {
		progress.update();
		interval = static_cast<int>(timer.get() / 2);
	}
	for (auto i{0}; i < order; ++i) { sprite.setTextureRect(sf::IntRect{{lookup, progress.get_count()}, {1, 1}}); }
}

} // namespace fornani::util
