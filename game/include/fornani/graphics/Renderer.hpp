
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/graphics/SpriteBatch.hpp>
#include <fornani/utils/Constants.hpp>

namespace fornani {
class Renderer {
  public:
	Renderer();
	void begin(sf::RenderWindow& win, sf::Vector2f cam);
	void submit(sf::Texture const& texture, sf::FloatRect dest, sf::IntRect uv, float scale = constants::f_scale_factor, sf::Color color = sf::Color::White, util::BitFlags<SpriteTransform> transform = {},
				RenderLayer layer = RenderLayer::particles);
	void submit(sf::Texture const& texture, sf::FloatRect dest, sf::IntRect uv, RenderLayer layer);
	void submit(sf::FloatRect dest, sf::IntRect uv, float scale = constants::f_scale_factor, sf::Color color = sf::Color::White, RenderLayer layer = RenderLayer::particles);
	void end();
	void flush();

  private:
	SpriteBatch m_batch{};
	std::vector<RenderCommand> m_commands{};
	sf::RenderWindow* m_target{};
	sf::Texture const* m_current_texture{};
	sf::Image img;
	sf::Texture m_white_texture{};
};

} // namespace fornani
