#pragma once
#include "Console.hpp"
#include "Selector.hpp"

namespace player {
class Player;
}

namespace gui {

class PauseWindow : public Console {
  public:
	PauseWindow() = default;
	PauseWindow(automa::ServiceProvider& svc);
	void update(automa::ServiceProvider& svc, player::Player& player);
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
		sf::Vector2<float> title_offset{(float)corner_factor, 16.f};
	} ui{};

	sf::Text title{};
	sf::Font title_font{};
	text::HelpText help_marker;

	sf::Text widget_label{};
	sf::Font widget_font{};
};

} // namespace gui
