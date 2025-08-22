
#include <editor/metagrid/Workspace.hpp>

namespace pi {

Workspace::Workspace(sf::Vector2u dimensions) {
	for (auto i = 0; i < dimensions.x * dimensions.y; ++i) { m_points.push_back(Point{i, dimensions}); }
}

void Workspace::render(sf::RenderWindow& win, sf::Vector2f cam) {
	for (auto& p : m_points) { p.render(win, cam); }
}

} // namespace pi
