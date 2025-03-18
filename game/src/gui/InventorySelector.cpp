
#include "fornani/gui/InventorySelector.hpp"
#include "fornani/service/ServiceProvider.hpp"

#include <ccmath/ext/clamp.hpp>

namespace fornani::gui {

InventorySelector::InventorySelector(sf::Vector2i range, sf::Vector2f spacing) : m_selection{util::Circuit{range.x}, util::Circuit{range.y}}, m_table_dimensions{range}, m_spacing{spacing} { m_body.physics.set_global_friction(0.8f); };

void InventorySelector::update() { m_body.update(); }

void InventorySelector::render(sf::RenderWindow& win, sf::Sprite& sprite, sf::Vector2f cam, sf::Vector2f origin) { m_body.constituent.render(win, sprite, cam, origin); }

void InventorySelector::set_position(sf::Vector2f to_position, bool force) {
	if (force) {
		m_body.physics.position = to_position;
	} else {
		m_body.steering.target(m_body.physics, get_menu_position() + to_position, 0.02f);
	}
}

void InventorySelector::move(sf::Vector2i direction) {
	m_selection[0].modulate(direction.x);
	m_selection[1].modulate(direction.y);
}

} // namespace fornani::gui
