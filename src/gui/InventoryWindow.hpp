
#pragma once
#include "Console.hpp"
#include "Selector.hpp"

namespace gui {

class InventoryWindow : public Console {
  public:
	InventoryWindow() = default;
	InventoryWindow(automa::ServiceProvider& svc);
	void update(automa::ServiceProvider& svc, player::Player& player);
	void render(automa::ServiceProvider& svc, player::Player& player, sf::RenderWindow& win);
	void open();
	void close();

	Selector selector{{2, 1}};

  private:
	struct {
		float corner_pad{120.f};
		int title_size{16};
		int items_per_row{12};
		sf::Vector2<float> title_offset{(float)corner_factor, 16.f};
	} ui{};

	sf::Text title{};
	sf::Font title_font{};
};

} // namespace gui
