
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/core/Fwd.hpp>
#include <fornani/io/Logger.hpp>

namespace fornani {

class FilterShader {
  public:
	FilterShader(ResourceFinder& finder, sf::Vector2u size);
	void finalize(float time, sf::Vector2u size, sf::Vector2f position, sf::Color highlight, float intensity = 0.f);
	void submit(sf::RenderWindow& win);
	void draw(sf::Drawable const& target);

  private:
	sf::Shader m_shader{};
	io::Logger m_logger{"shader"};
	sf::RenderTexture m_texture{};
};

} // namespace fornani
