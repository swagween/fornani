
#include "ColorFade.hpp"

namespace vfx {
ColorFade::ColorFade(std::vector<sf::Color> colors, int threshold, int duration) : colors(colors), threshold(threshold), duration(duration) {}

void ColorFade::start() { cooldown.start(duration); }

void ColorFade::update() {
	cooldown.update();
	auto diff = duration - cooldown.get_cooldown();
	if (colors.size() < 3) { return; }
	current_color = colors.at(1);
	if (diff < threshold) { current_color = colors.at(0); }
	if (diff > duration - threshold) { current_color = colors.at(2); }
}

} // namespace vfx
