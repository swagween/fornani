
#pragma once
#include <SFML/Graphics.hpp>
#include <string_view>
#include "fornani/utils/Circuit.hpp"
#include "fornani/utils/BitFlags.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::gui {

enum class InventorySection { item, gun, END };
enum class SelectorFlags { switched, went_up };

class Selector {
  public:
	Selector(automa::ServiceProvider& svc, sf::Vector2<int> dim);
	void update();
	void render(sf::RenderWindow& win) const;
	void switch_sections(sf::Vector2<int> way);
	void go_up(bool has_arsenal = false);
	void go_down(bool has_arsenal = false);
	void go_left();
	void go_right();
	void set_size(int size);
	void set_dimensions(sf::Vector2<int> dim);
	void set_position(sf::Vector2<float> pos) { position = pos; }
	bool last_row() const;
	[[nodiscard]] auto switched_sections() -> bool { return flags.consume(SelectorFlags::switched); }
	[[nodiscard]] auto get_section() const -> InventorySection { return section; }
	[[nodiscard]] auto get_section_int() const -> int { return static_cast<int>(section); }
	[[nodiscard]] auto get_current_selection() const -> int { return current_selection.get(); }
	[[nodiscard]] auto get_menu_position() const -> sf::Vector2<float> { return position + sf::Vector2<float>{2.f * sprite.getLocalBounds().size.x, -8.f}; }

	util::Circuit current_selection{1};

  private:
	automa::ServiceProvider* m_services;
	InventorySection section{};
	util::BitFlags<SelectorFlags> flags{};

	sf::Vector2<int> table_dimensions{};
	sf::Vector2<float> position{};
	sf::Sprite sprite;

	struct {
		util::Circuit vertical{static_cast<int>(InventorySection::END)};
	} sections{};
};

} // namespace gui
