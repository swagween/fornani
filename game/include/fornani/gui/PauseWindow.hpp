#pragma once
#include "Console.hpp"
#include "MiniMenu.hpp"
#include "Selector.hpp"

namespace player {
class Player;
}

namespace automa {
class GameState;
}

namespace gui {

class PauseWindow : public Console {
  public:
	PauseWindow(automa::ServiceProvider& svc);
	void update(automa::ServiceProvider& svc, Console& console, bool automatic);
	void render_update(automa::ServiceProvider& svc);
	void render(automa::ServiceProvider& svc, player::Player& player, sf::RenderWindow& win);
	void open();
	void close();

	Selector selector;

  private:
	struct {
		float corner_pad{410.f};
		int title_size{16};
		int widget_size{16};
		sf::Vector2<float> widget_label_offset{(float)corner_factor, 48.f};
		sf::Vector2<float> title_offset{0.f, -40.f};
	} ui{};

	sf::Text title;
	sf::Text widget_label;
	text::HelpText help_marker;

	MiniMenu menu;
};

} // namespace gui
