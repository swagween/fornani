
#pragma once

#include <vector>
#include "Spark.hpp"

namespace vfx {

class Sparkler {
  public:
	Sparkler() = default;
	Sparkler(sf::Vector2<float> dimensions, sf::Color color) : dimensions(dimensions), color(color) {};
	void update();
	void render(sf::RenderWindow& win, sf::Vector2<float> cam);
	void set_position(sf::Vector2<float> pos);
	void set_dimensions(sf::Vector2<float> dim);

  private:
	std::vector<Spark> sparkles{};
	sf::Vector2<float> dimensions{};
	sf::Vector2<float> position{};

	sf::Color color{};
};

} // namespace vfx
