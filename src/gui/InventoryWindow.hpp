
#pragma once
#include "Console.hpp"
#include "Selector.hpp"
#include "MiniMap.hpp"
#include "MiniMenu.hpp"
#include "WardrobeWidget.hpp"

namespace player {
class Player;
}

namespace item {
class Item;
}

namespace gui {

enum class Mode{inventory, minimap};

class InventoryWindow : public Console {
  public:
	InventoryWindow() = default;
	InventoryWindow(automa::ServiceProvider& svc);
	void update(automa::ServiceProvider& svc, player::Player& player, world::Map& map);
	void render(automa::ServiceProvider& svc, player::Player& player, sf::RenderWindow& win, sf::Vector2<float> cam);
	void open(automa::ServiceProvider& svc, player::Player& player);
	void update_wardrobe(automa::ServiceProvider& svc, player::Player& player);
	void close();
	void select();
	void cancel();
	void move(sf::Vector2<int> direction);
	void use_item(automa::ServiceProvider& svc, player::Player& player, world::Map& map, item::Item& item);
	void update_table(player::Player& player, bool new_dim);
	void set_item_size(int sz) {
		if (sz > 0) { selector.current_selection = util::Circuit(sz); }
	}
	void switch_modes(automa::ServiceProvider& svc);
	[[nodiscard]] auto is_inventory() const -> bool { return mode == Mode::inventory; }
	[[nodiscard]] auto is_minimap() const -> bool { return mode == Mode::minimap; }
	[[nodiscard]] auto get_wardrobe_sprite() -> sf::Sprite& { return wardrobe.get_sprite(); }

	Selector selector;
	Console info;
	MiniMap minimap;
	Mode mode{};
	MiniMenu item_menu;
	text::HelpText help_marker;

  private:
	struct {
		float corner_pad{static_cast<float>(-corner_factor) * 1.5f};
		float inner_corner{86.f};
		float buffer{40.f};
		int title_size{16};
		int desc_size{16};
		sf::Vector2<float> title_offset{static_cast<float>(corner_factor) * 1.3f, 66.f};
		sf::Vector2<float> arsenal_offset{static_cast<float>(corner_factor) * 1.3f, 266.f};
		sf::Vector2<float> item_label_offset{static_cast<float>(corner_factor) * 1.3f, 390.f};
		sf::Vector2<float> item_description_offset{static_cast<float>(corner_factor) * 1.3f, 490.f};
		sf::Vector2<float> arsenal_position{static_cast<float>(corner_factor) * 1.3f, 280.f};
		sf::Vector2<float> info_offset{66.f, 180.f};
		sf::Vector2<float> rarity_pad{32.f, 32.f};
		sf::Vector2<float> wardrobe_offset{380.f, -32.f};
	} ui{};

	sf::Text title{};
	sf::Font title_font{};
	sf::Text arsenal{};

	sf::Text item_label{};
	sf::Font item_font{};

	sf::CircleShape gun_slot{};

	WardrobeWidget wardrobe{};
};

} // namespace gui
