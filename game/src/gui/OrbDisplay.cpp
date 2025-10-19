
#include <fornani/gui/OrbDisplay.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani::gui {

OrbDisplay::OrbDisplay(automa::ServiceProvider& svc) : Animatable(svc, "orbs", {12, 12}), m_amount{svc.text.fonts.title} {
	set_parameters(anim::Parameters{0, 7, 24, -1});
	m_amount.setCharacterSize(16);
}

void OrbDisplay::update(int amount) {
	m_amount.setString(std::to_string(amount));
	tick();
}

void OrbDisplay::render(sf::RenderWindow& win, sf::Vector2f const pos) {
	set_position(pos);
	auto text_offset = sf::Vector2f{46.f, 4.f};
	m_amount.setPosition(pos + text_offset);
	m_amount.setFillColor(colors::pioneer_red);
	win.draw(m_amount);
	win.draw(*this);
}

} // namespace fornani::gui
