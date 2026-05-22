
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/graphics/SpriteBatch.hpp>
#include <fornani/utils/Constants.hpp>

namespace fornani {
class Renderer {
  public:
	Renderer();
	void begin(sf::RenderWindow& win, sf::Vector2f cam);
	void submit(sf::Texture const& texture, sf::FloatRect dest, sf::IntRect uv, float scale = constants::f_scale_factor, sf::Color color = sf::Color::White);
	void submit(sf::FloatRect dest, sf::IntRect uv, float scale = constants::f_scale_factor, sf::Color color = sf::Color::White);
	void end() { flush(); }

  private:
	void flush();

  private:
	SpriteBatch m_batch{};
	sf::RenderWindow* m_target{};
	sf::Texture const* m_current_texture{};
	sf::Image img;
	sf::Texture m_white_texture{};
};

} // namespace fornani
