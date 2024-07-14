
#pragma once
#include "Console.hpp"
#include "Selector.hpp"
#include "MiniMap.hpp"

namespace player {
class Player;
}

namespace gui {

	enum class Mode{inventory, minimap};

class InventoryWindow : public Console {
  public:
	InventoryWindow() = default;
	InventoryWindow(automa::ServiceProvider& svc);
	void update(automa::ServiceProvider& svc, player::Player& player, world::Map& map);
	void render(automa::ServiceProvider& svc, player::Player& player, sf::RenderWindow& win, sf::Vector2<float> cam);
	void open();
	void close();
	void set_item_size(int sz) {
		if (sz > 0) { selector.current_selection = util::Circuit(sz); }
	}
	void switch_modes(automa::ServiceProvider& svc);
	[[nodiscard]] auto is_inventory() const -> bool { return mode == Mode::inventory; }
	[[nodiscard]] auto is_minimap() const -> bool { return mode == Mode::minimap; }

	Selector selector;
	Console info;
	MiniMap minimap;
	Mode mode{};
	text::HelpText help_marker;

  private:
	struct {
		float corner_pad{120.f};
		float inner_corner{16.f};
		int title_size{16};
		int desc_size{16};
		int items_per_row{12};
		sf::Vector2<float> title_offset{(float)corner_factor, 16.f};
		sf::Vector2<float> item_label_offset{(float)corner_factor, 230.f};
		sf::Vector2<float> item_description_offset{(float)corner_factor, 290.f};
		sf::Vector2<float> info_offset{inner_corner, 260.f};
	} ui{};

	sf::Text title{};
	sf::Font title_font{};

	sf::Text item_label{};
	sf::Font item_font{};
};

} // namespace gui
