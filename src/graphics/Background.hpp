
#pragma once

#include <SFML/Graphics.hpp>
#include <array>
#include <unordered_map>
#include "../components/PhysicsComponent.hpp"


namespace automa {
struct ServiceProvider;
}

namespace bg {

struct BackgroundLayer {
	int render_layer{};
	float scroll_speed{};
	float parallax{};
	sf::Sprite sprite{};
	components::PhysicsComponent physics{};
};

class Background {
  public:
	Background() = default;
	Background(automa::ServiceProvider& svc, int bg_id);

	void update(automa::ServiceProvider& svc, sf::Vector2<float> observed_camvel);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float>& campos, sf::Vector2<float>& mapdim);
	void debug();

  private:
	std::vector<BackgroundLayer> layers{};
	sf::Vector2<int> scroll_pane{};
	sf::Vector2<int> dimensions{};
	sf::Vector2<int> start_offset{};
	bool* b_debug{};
	std::unordered_map<int, std::string_view> labels{};
};

} // namespace bg
