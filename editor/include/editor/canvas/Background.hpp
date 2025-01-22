
#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <string>

namespace pi {

class ResourceFinder;
class Canvas;

struct BackgroundLayer {
	int render_layer{};
	float scroll_speed{};
	float parallax{};
	sf::Sprite sprite{};
	sf::Vector2<float> position{};
	sf::Vector2<float> velocity{};
	sf::Vector2<float> final_position{};
};

class Background {
  public:
	Background() = default;
	Background(ResourceFinder& finder, int bg_id);

	void update();
	void render(Canvas& canvas, sf::RenderWindow& win, sf::Vector2<float>& campos);
	void debug();

  private:
	std::vector<BackgroundLayer> layers{};
	sf::Vector2<int> scroll_pane{};
	sf::Vector2<int> dimensions{};
	sf::Vector2<int> start_offset{};
	bool* b_debug{};
	std::unordered_map<int, std::string> labels{};
	sf::Texture texture{};
};

} // namespace bg
