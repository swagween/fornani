
#include <editor/metagrid/Workspace.hpp>

namespace pi {

constexpr auto grid_offset_v = sf::Vector2f{-1.f, -1.f};

Workspace::Workspace(sf::Vector2u dimensions) {
	m_texture.clear();
	if (!m_texture.resize(dimensions * static_cast<std::uint32_t>(spacing_v))) { NANI_LOG_WARN(m_logger, "Failed to resize map texture"); }
	for (auto i = 0; i < dimensions.x * dimensions.y; ++i) {
		m_points.push_back(Point{i, dimensions});
		m_texture.draw(m_points.back().dot);
	}
	m_texture.display();
}

void Workspace::render(sf::RenderWindow& win, sf::Vector2f cam) {
	auto sprite = sf::Sprite{m_texture.getTexture()};
	sprite.setPosition(cam + grid_offset_v);
	win.draw(sprite);
}

} // namespace pi
