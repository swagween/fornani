
#pragma once

#include <SFML/Graphics.hpp>
#include <array>
#include "fornani/gui/Gizmo.hpp"
#include "fornani/utils/Circuit.hpp"
#include "fornani/utils/Constants.hpp"

namespace fornani::gui {

class InventorySelector {
	friend class OutfitterGizmo;
	friend class InventoryGizmo;
	friend class VendorDialog;

  public:
	InventorySelector(sf::Vector2i range, sf::Vector2f spacing = constants::f_cell_vec);
	void update();
	void render(sf::RenderWindow& win, sf::Sprite& sprite, sf::Vector2f cam, sf::Vector2f origin);

	[[nodiscard]] auto get_current_selection() const -> int { return m_selection[0].get() + m_selection[1].get() * m_table_dimensions.x; }
	[[nodiscard]] auto get_current_selection(int dim) const -> int { return m_selection[0].get() + m_selection[1].get() * dim; }
	[[nodiscard]] auto get_index() const -> sf::Vector2i { return sf::Vector2i{m_selection[0].get(), m_selection[1].get()}; }
	[[nodiscard]] auto get_horizonal_index() const -> int { return m_selection[0].get(); }
	[[nodiscard]] auto get_vertical_index() const -> int { return m_selection[1].get(); }
	[[nodiscard]] auto get_spacing() const -> sf::Vector2f { return m_spacing; };
	[[nodiscard]] auto get_position() const -> sf::Vector2f { return m_body.physics.position; };
	[[nodiscard]] auto get_menu_position() const -> sf::Vector2f { return sf::Vector2f{m_selection[0].as<float>(), m_selection[1].as<float>()}.componentWiseMul(m_spacing); };

  private:
	void set_lookup(sf::IntRect to_lookup) { m_body.constituent.lookup = to_lookup; }
	void set_position(sf::Vector2f to_position, bool force = false);
	void move(sf::Vector2i direction);
	void set_selection(sf::Vector2i to_selection);

	Direction move_direction(sf::Vector2i direction);
	FreeConstituent m_body{};
	std::array<util::Circuit, 2> m_selection;
	sf::Vector2i m_table_dimensions{};
	sf::Vector2f m_spacing{};
};

} // namespace fornani::gui
