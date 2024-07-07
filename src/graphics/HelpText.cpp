#include "HelpText.hpp"
#include "HelpText.hpp"
#include "../service/ServiceProvider.hpp"

namespace text {

void HelpText::init(automa::ServiceProvider& svc, std::string start, std::string_view code, std::string end) {
	font.loadFromFile(svc.text.text_font);
	font.setSmooth(false);
	text_color = svc.styles.colors.ui_white;
	text_color.a = 0;
	data.setCharacterSize(text_size);
	data.setFont(font);
	data.setLineSpacing(1.5f);
	marker = start + svc.controller_map.tag_to_label.at(code).data() + end;
	data.setString(marker);
	data.setFont(font);
	data.setCharacterSize(text_size);
	data.setOrigin(data.getLocalBounds().getSize() * 0.5f);
	data.setPosition({static_cast<float>(svc.constants.screen_dimensions.x) * 0.5f, static_cast<float>(svc.constants.screen_dimensions.y) - 2.f * pad});
}

void HelpText::render(sf::RenderWindow& win) {
	delay.update();
	if (!ready()) { return; }
	alpha_counter.update();
	auto alpha = static_cast<int>(-128 * cos(0.06f * alpha_counter.get_count()) + 128);
	text_color.a = std::clamp(alpha, 0, 255);
	data.setFillColor(text_color);
	win.draw(data);
}


void HelpText::set_color(sf::Color color) { text_color = color; }

void HelpText::set_string(std::string string) { data.setString(string); }

void HelpText::set_alpha(uint8_t a) { text_color.a = a; }

void HelpText::reset() {
	text_color.a = 0;
	alpha_counter.start();
}
} // namespace text
