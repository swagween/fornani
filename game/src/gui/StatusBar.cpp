
#include "fornani/gui/StatusBar.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::gui {

StatusBar::StatusBar(automa::ServiceProvider& svc, sf::Vector2f dim, std::vector<sf::Color> colors, bool centered) : m_dimensions(dim) {
	m_rects.filled.setFillColor(colors[0]);
	m_rects.taken.setFillColor(colors[1]);
	m_rects.gone.setFillColor(colors[2]);

	m_rects.filled.setSize(m_dimensions);
	m_rects.taken.setSize(m_dimensions);
	m_rects.gone.setSize(m_dimensions);

	if (centered) {
		m_rects.gone.setOrigin(m_rects.gone.getLocalBounds().getCenter());
		m_rects.filled.setOrigin(m_rects.gone.getLocalBounds().getCenter());
		m_rects.taken.setOrigin(m_rects.gone.getLocalBounds().getCenter());
	}
}

void StatusBar::update(automa::ServiceProvider& svc, sf::Vector2f position, entity::Health& status, bool ease) { update(svc, position, status.get_normalized(), status.get_taken_point()); }

void StatusBar::update(automa::ServiceProvider& svc, sf::Vector2f position, float fraction, float taken, bool ease) {

	set_position(position);
	if (ease) {
		m_steering.seek(m_physics, position);
	} else {
		m_physics.position = position;
	}
	m_physics.update(svc);

	m_rects.filled.setSize({m_dimensions.x * fraction, m_dimensions.y});
	m_rects.taken.setSize({m_dimensions.x * taken, m_dimensions.y});
	m_rects.gone.setSize(m_dimensions);
}

void StatusBar::render(sf::RenderWindow& win) {
	win.draw(m_rects.gone);
	win.draw(m_rects.taken);
	win.draw(m_rects.filled);
}

void StatusBar::set_origin(sf::Vector2f origin) {
	m_rects.gone.setOrigin(origin);
	m_rects.taken.setOrigin(origin);
	m_rects.filled.setOrigin(origin);
}

void StatusBar::set_position(sf::Vector2f position) {
	m_rects.gone.setPosition(position);
	m_rects.taken.setPosition(position);
	m_rects.filled.setPosition(position);
}

} // namespace fornani::gui
