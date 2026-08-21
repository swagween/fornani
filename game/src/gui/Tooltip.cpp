
#include <fornani/gui/Tooltip.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani::gui {

Tooltip::Tooltip(automa::ServiceProvider& svc, std::string_view style, std::string_view tag, sf::Vector2f anchor)
	: m_nineslice{svc, svc.assets.get_texture("tooltip_" + std::string{style}), {4, 4}, {1, 1}}, m_anchor{anchor}, m_text{svc.text.fonts.basic.font}, m_start{60} {
	m_text.setFillColor(colors::pioneer_red);
	m_text.setCharacterSize(8);
	m_text.setString(svc.data.tooltips[tag]["message"].as_string());
	m_nineslice.set_position(anchor + random::random_vector_float(-32.f, 32.f));
	m_start.start();
}

void Tooltip::update() {
	m_start.update();
	if (m_start.running()) { return; }
	m_nineslice.target_position(m_anchor + sf::Vector2f{14.f, 14.f});
}

void Tooltip::render(sf::RenderWindow& win, sf::Vector2f cam, sf::Vector2f scale) {
	if (m_start.running()) { return; }
	m_nineslice.set_scale(scale);
	m_text.setScale(scale);
	m_nineslice.set_dimensions(m_text.getLocalBounds().size.componentWiseMul(scale));
	m_text.setPosition(m_nineslice.get_position());
	m_nineslice.render(win);
	win.draw(m_text);
}

} // namespace fornani::gui
