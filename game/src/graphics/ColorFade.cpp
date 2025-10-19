
#include "fornani/graphics/ColorFade.hpp"

namespace fornani::vfx {

ColorFade::ColorFade(std::vector<sf::Color> colors, int threshold, int duration) : colors(colors), threshold(threshold), duration(duration) {}

void ColorFade::change_colors(std::vector<sf::Color> to_colors) { colors = to_colors; }

void ColorFade::start() { cooldown.start(duration); }

void ColorFade::update() {
	cooldown.update();
	auto diff = duration - cooldown.get();
	if (colors.size() < 3) { return; }
	current_color = colors.at(1);
	if (diff < threshold) { current_color = colors.at(0); }
	if (diff > duration - threshold) { current_color = colors.at(2); }
}

} // namespace fornani::vfx
