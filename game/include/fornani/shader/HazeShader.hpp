
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/core/Fwd.hpp>
#include <fornani/io/Logger.hpp>

namespace fornani {

class HazeShader {
  public:
	HazeShader(ResourceFinder& finder, sf::Vector2u size, float parallax = 0.f);
	void finalize(float time, sf::Vector2u size, sf::Color highlight, sf::Color shadow, sf::Vector2f cam);
	void submit(sf::RenderWindow& win, sf::Drawable const& target, sf::Vector2f cam);

  private:
	sf::Shader m_shader{};
	io::Logger m_logger{"shader"};
	sf::RenderTexture m_texture{};
	float m_parallax{};
};

} // namespace fornani
