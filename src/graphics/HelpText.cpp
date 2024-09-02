#include "HelpText.hpp"
#include "../service/ServiceProvider.hpp"
#include "HelpText.hpp"

namespace text {

void HelpText::init(automa::ServiceProvider& svc, std::string start, config::DigitalAction const& code, std::string end, int delay_time, bool include_background) {
	font.loadFromFile(svc.text.text_font);
	font.setSmooth(false);
	text_color = svc.styles.colors.ui_white;
	text_color.a = 0;
	bg_color = svc.styles.colors.ui_black;
	bg_color.a = 0;
	data.setCharacterSize(text_size);
	data.setFont(font);
	data.setLineSpacing(1.5f);
	marker = start + svc.controller_map.digital_action_name(code).data() + end;
	data.setString(marker);
	data.setFont(font);
	data.setCharacterSize(text_size);
	data.setOrigin(data.getLocalBounds().getSize() * 0.5f);
	position = {static_cast<float>(svc.constants.screen_dimensions.x) * 0.5f, static_cast<float>(svc.constants.screen_dimensions.y) - 2.f * pad};
	data.setPosition(position);
	delay.start(delay_time);
	background = include_background;
}

void HelpText::render(sf::RenderWindow& win) {
	delay.update();
	if (!ready()) { return; }
	alpha_counter.update();
	auto alpha = static_cast<int>(-128 * cos(0.06f * alpha_counter.get_count()) + 128);
	text_color.a = std::clamp(alpha, 0, 255);
	bg_color.a = std::clamp(alpha, 0, 255);
	if (background) {
		data.setPosition(position + bg_offset);
		data.setFillColor(bg_color);
		win.draw(data);
	}
	data.setPosition(position);
	data.setFillColor(text_color);
	win.draw(data);
}

void HelpText::set_color(sf::Color color) { text_color = color; }

void HelpText::set_string(std::string string) { data.setString(string); }

void HelpText::set_alpha(uint8_t a) { text_color.a = a; }

void HelpText::set_position(sf::Vector2<float> pos) { position = pos; }

void HelpText::reset() {
	text_color.a = 0;
	alpha_counter.start();
}
} // namespace text
