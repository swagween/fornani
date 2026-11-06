
#pragma once

#include <SFML/Graphics.hpp>
#include <djson/json.hpp>
#include <fornani/core/Fwd.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/utils/Circuit.hpp>
#include <array>

namespace fornani::vfx {

struct WeatherParameters {
	WeatherParameters(dj::Json const& in);
	int density{1};
	int framerate{8};
	float parallax_factor{0.05f};
	std::optional<std::string> effect{};
	std::optional<std::string> ambience{};
};

struct WeatherLayer {
	std::vector<sf::RenderTexture> textures{};
	float parallax{};
	float seed{};
	std::size_t render_order{};
	util::Circuit loop;
	sf::Vector2f position{};
};

class Weather {
  public:
	explicit Weather(automa::ServiceProvider& svc, world::Map& map, std::string_view tag);
	void update(automa::ServiceProvider& svc, world::Map& map);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam, std::size_t layer);

	[[nodiscard]] auto get_parameters() const -> WeatherParameters const& { return m_parameters; }

  private:
	WeatherParameters m_parameters;
	std::vector<WeatherLayer> m_layers{};
	sf::IntRect m_rect{};

	io::Logger m_logger{"Weather"};
};

} // namespace fornani::vfx
