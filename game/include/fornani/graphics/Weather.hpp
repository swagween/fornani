
#pragma once

#include <SFML/Graphics.hpp>
#include <djson/json.hpp>
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
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam, std::size_t layer);

  private:
	WeatherParameters m_parameters{};
	std::array<WeatherLayer, 3> m_layers{};
	sf::IntRect m_rect{};

	io::Logger m_logger{"Weather"};
};

// for pioneer
struct WeatherSpecifications {
	WeatherSpecifications();
	WeatherSpecifications(dj::Json const& in);
	void serialize(dj::Json& out);
	std::string type{};
	std::string effect{};
	std::string ambience{};
	WeatherParameters params{};
	float chance{};
};

} // namespace fornani::vfx
