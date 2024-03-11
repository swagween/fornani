#pragma once
#include <imgui.h>
#include <random>
#include "../automa/StateManager.hpp"
#include "../service/ServiceProvider.hpp"
#include <imgui-SFML.h>

namespace fornani {

class Game {
  public:
	Game() = default;
	Game(char** argv);
	void run();

  private:
	void debug_window();
	void take_screenshot();
	bool debug();

	//currently unused. replace existing inline services with these to clean tech debt.
	struct automa::ServiceProvider services{};

	struct {
		sf::Vector2<uint32_t> win_size{};
		float height_ratio{};
		float width_ratio{};
	} measurements{};

	struct {
		int draw_calls{};
	} trackers{};

	automa::StateManager game_state{};
	sf::RenderWindow window{};
	sf::Texture screencap{};
	sf::RectangleShape background{};
};

} // namespace fornani