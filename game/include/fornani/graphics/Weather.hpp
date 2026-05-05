
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/core/Fwd.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/utils/Circuit.hpp>
#include <array>

namespace fornani::vfx {

struct WeatherParameters {
	int density{};
	int framerate{};
};

struct WeatherLayer {
	std::array<sf::RenderTexture, 16> textures{};
	float parallax{};
	util::Circuit loop{16};
};

class Weather {
  public:
	explicit Weather(automa::ServiceProvider& svc, world::Map& map, WeatherParameters params);
	void update(automa::ServiceProvider& svc, world::Map& map);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);

  private:
	WeatherParameters m_parameters{};
	std::array<WeatherLayer, 3> m_layers{};
	sf::IntRect m_rect{};

	io::Logger m_logger{"Weather"};
};

} // namespace fornani::vfx
