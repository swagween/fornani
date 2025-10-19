
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/io/Logger.hpp>
#include <string_view>
#include <vector>

namespace fornani {

class ResourceFinder;

class Palette {
  public:
	Palette(std::string_view source, ResourceFinder& finder);
	void render(sf::RenderWindow& win);
	void set_position(sf::Vector2f to_position);
	sf::RenderTexture& get_texture();
	[[nodiscard]] auto get_size() const -> std::size_t { return m_swatches.size(); }

  private:
	std::vector<sf::Color> m_swatches{};
	sf::RenderTexture m_texture{};

	io::Logger m_logger{"shader"};
};

} // namespace fornani
