
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/graphics/SpriteBatch.hpp>

namespace fornani {
class Renderer {
  public:
	void begin(sf::RenderWindow& win, sf::Vector2f cam);
	void submit(sf::Texture const& texture, sf::FloatRect dest, sf::IntRect uv, sf::Color color = sf::Color::White);
	void end() { flush(); }

  private:
	void flush();

  private:
	SpriteBatch m_batch{};
	sf::RenderWindow* m_target{};
	sf::Texture const* m_current_texture{};
};

} // namespace fornani
