
#include <fornani/gui/DottedLine.hpp>

namespace fornani::gui {

DottedLine::DottedLine(std::pair<sf::Vector2f, sf::Vector2f> segment, float spacing, DotProperties properties) : m_segment{segment}, m_spacing{spacing}, m_properties{properties} {
	auto const start = segment.first;
	auto const end = segment.second;

	auto delta = end - start;
	auto length = std::sqrt(delta.x * delta.x + delta.y * delta.y);

	if (length == 0.f || spacing <= 0.f) { return; }

	auto direction = delta / length;

	for (auto distance = 0.f; distance <= length; distance += spacing) {
		auto position = start + direction * distance;
		create_dot(position, properties);
	}
}

void DottedLine::update() {
	auto const start = m_segment.first;
	auto const end = m_segment.second;

	auto delta = end - start;
	auto length = std::sqrt(delta.x * delta.x + delta.y * delta.y);

	if (length == 0.f || m_spacing <= 0.f) {
		m_dots.clear();
		return;
	}

	auto direction = delta / length;

	auto required_dot_count = static_cast<std::size_t>(length / m_spacing) + 1;

	m_dots.resize(required_dot_count);

	for (std::size_t i = 0; i < required_dot_count; ++i) {
		auto distance = i * m_spacing;
		sf::Vector2f position = start + direction * distance;

		sf::RectangleShape& dot = m_dots[i].rect;
		dot.setFillColor(m_properties.color);
		dot.setSize({m_properties.size, m_properties.size});
		dot.setOrigin(dot.getLocalBounds().getCenter());
		m_dots[i].position = position;
	}
}

void DottedLine::render(sf::RenderWindow& win, float scale, sf::Vector2f position, sf::Vector2f viewport_size) {
	for (auto& dot : m_dots) {
		dot.rect.setPosition(((dot.position * scale) + position).componentWiseDiv(viewport_size));
		win.draw(dot.rect);
	}
}

void DottedLine::create_dot(sf::Vector2f const position, DotProperties const& properties) {
	auto dot = sf::RectangleShape{};
	dot.setFillColor(properties.color);
	dot.setSize({properties.size, properties.size});
	dot.setOrigin(dot.getLocalBounds().getCenter());
	m_dots.push_back(Dot{dot, position});
}

} // namespace fornani::gui
