
#include <fornani/graphics/HelpText.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <algorithm>

namespace fornani::graphics {

HelpText::HelpText(automa::ServiceProvider& svc) : data(svc.text.fonts.title.font) { alpha_counter.set_cycle_point(std::numbers::pi * 2.f); }

HelpText::HelpText(automa::ServiceProvider& svc, std::string start, input::DigitalAction const& code, std::string end, int delay_time, bool include_background, bool no_blink) : HelpText(svc) {
	text_color = colors::ui_white;
	text_color.a = 0;
	bg_color = colors::black;
	bg_color.a = 0;
	data.setCharacterSize(text_size);
	data.setLineSpacing(1.5f);

	auto const prefix = start;
	auto const key = std::string{svc.input_system.get_digital_action_source_name(code)};
	auto const suffix = end;

	m_key_start = start.size();
	m_key_end = m_key_start + key.size();

	marker = start + key + end;
	data.setString(marker);

	auto& vertices = data.getVertexData();
	auto const& shaped_glyphs = data.getShapedGlyphs();

	auto const first_cluster = prefix.size();
	auto const last_cluster = first_cluster + key.size();

	for (auto const& glyph : shaped_glyphs) {
		if (glyph.cluster < first_cluster || glyph.cluster >= last_cluster) { continue; }

		for (auto index = glyph.vertexOffset; index < glyph.vertexOffset + glyph.vertexCount; ++index) { vertices[index].color = colors::bright_orange; }
	}

	data.setCharacterSize(text_size);
	data.setOrigin(data.getLocalBounds().getCenter());
	position = {static_cast<float>(svc.window->i_screen_dimensions().x) * 0.5f, static_cast<float>(svc.window->i_screen_dimensions().y) - 2.f * pad};
	data.setPosition(position);
	delay.start(delay_time);
	background = include_background;
	if (no_blink) {
		flags.set(HelpTextFlags::no_blink);
		delay.cancel();
	}
}

void HelpText::update() {
	delay.update();
	if (!ready()) { return; }
	alpha_counter.update(0.02f);
}

void HelpText::render(sf::RenderWindow& win) {
	if (!ready()) { return; }

	auto alpha = static_cast<unsigned>(-128 * sin(alpha_counter.get()) + 128);
	if (flags.test(HelpTextFlags::no_blink)) { alpha = 255u; }

	text_color.a = std::clamp(alpha, 0u, 255u);
	bg_color.a = std::clamp(alpha, 0u, 255u);

	if (background) {
		data.setPosition(position + bg_offset);
		data.setFillColor(bg_color);
		win.draw(data);
	}

	data.setPosition(position);
	data.setFillColor(text_color);

	auto& vertices = data.getVertexData();
	auto const& glyphs = data.getShapedGlyphs();

	for (auto const& glyph : glyphs) {
		if (glyph.cluster < m_key_start || glyph.cluster >= m_key_end) { continue; }

		for (auto i = glyph.vertexOffset; i < glyph.vertexOffset + glyph.vertexCount; ++i) { vertices[i].color = sf::Color(255, 198, 28, text_color.a); }
	}

	win.draw(data);
}

void HelpText::set_color(sf::Color color) { text_color = color; }

void HelpText::set_string(std::string string) { data.setString(string); }

void HelpText::set_alpha(std::uint8_t a) { text_color.a = a; }

void HelpText::set_position(sf::Vector2f pos) { position = pos; }

void HelpText::reset() {
	text_color.a = 0;
	alpha_counter.reset();
}

} // namespace fornani::graphics
