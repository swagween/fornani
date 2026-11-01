#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/core/Fwd.hpp>
#include <fornani/io/Logger.hpp>

namespace fornani {

class SparkleShader {
  public:
	SparkleShader(ResourceFinder& finder, sf::Vector2u size);
	void finalize(float time, sf::Vector2u size, sf::Vector2f camera_position, sf::Texture const& white_noise, sf::Texture const& diagonal_mask, sf::Color highlight, float intensity = 0.f);
	void submit(sf::RenderWindow& win, sf::Vector2f cam);
	void draw(sf::Drawable const& target);
	void draw(sf::Texture const& texture);
	auto get_render_texture() -> sf::RenderTexture& { return m_texture; }

  private:
	sf::Shader m_shader{};
	io::Logger m_logger{"shader"};
	sf::RenderTexture m_texture{};
};

} // namespace fornani
