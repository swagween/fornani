
#pragma once

#include <vector>
#include "Spark.hpp"

namespace vfx {

class Sparkler {
  public:
	Sparkler() = default;
	Sparkler(automa::ServiceProvider& svc, sf::Vector2<float> dimensions = {2.f, 2.f}, sf::Color color = sf::Color::White, std::string_view type = "");
	void update(automa::ServiceProvider& svc);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);
	void set_position(sf::Vector2<float> pos);
	void set_dimensions(sf::Vector2<float> dim);
	void deactivate();
	void set_color(sf::Color to_color);
	void set_rate(float to_rate);

  private:
	std::vector<Spark> sparkles{};
	sf::Vector2<float> dimensions{};
	sf::Vector2<float> position{};
	std::string_view type{};

	struct {
		float rate{};
	} behavior{};

	sf::Color color{};
	sf::RectangleShape drawbox{}; // for debug
	bool active{true};
};

} // namespace vfx
