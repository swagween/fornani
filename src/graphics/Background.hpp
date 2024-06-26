
#pragma once

#include <SFML/Graphics.hpp>
#include <array>
#include <unordered_map>
#include "../components/PhysicsComponent.hpp"


namespace automa {
struct ServiceProvider;
}

namespace bg {

static int const scroll_size{1920};

class Background {

  public:
	Background() = default;
	Background(automa::ServiceProvider& svc, int bg_id);

	void update(automa::ServiceProvider& svc, sf::Vector2<float> observed_camvel);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float>& campos, sf::Vector2<float>& mapdim);

  private:
	std::vector<sf::Sprite> sprites{};
	sf::Vector2<int> dimensions{};
	sf::Vector2<int> start_offset{};
	struct {
		int used_layers{};
		float scroll_speed{};
		bool scrolling{};
		float parallax_multiplier{};
	} behavior{};

	std::vector<components::PhysicsComponent> physics{};
};

} // namespace bg
