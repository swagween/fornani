
#include <fornani/core/Debug.hpp>
#include <fornani/graphics/Renderer.hpp>
#include <fornani/utils/Constants.hpp>

namespace fornani {

Renderer::Renderer() {
	img.resize({1, 1}, sf::Color::White);
	if (!m_white_texture.loadFromImage(img)) {}
}

void Renderer::begin(sf::RenderWindow& win, sf::Vector2f cam) {
	m_target = &win;
	m_batch.clear();
	m_batch.setPosition(-cam);
	m_current_texture = nullptr;
}

void Renderer::submit(sf::Texture const& texture, sf::FloatRect dest, sf::IntRect uv, float scale, sf::Color color, util::BitFlags<SpriteTransform> transform, RenderLayer layer) {
	if (&texture != m_current_texture) {
		flush();
		m_current_texture = &texture;
		m_batch.setTexture(texture);
	}
	dest.size *= scale;

	m_batch.add(dest, uv, color, transform);
}

void Renderer::submit(sf::Texture const& texture, sf::FloatRect dest, sf::IntRect uv, RenderLayer layer) { submit(texture, dest, uv, constants::f_scale_factor, sf::Color::White, {}, layer); }

void Renderer::end() { flush(); }

void Renderer::submit(sf::FloatRect dest, sf::IntRect uv, float scale, sf::Color color, RenderLayer layer) { submit(m_white_texture, dest, uv, scale, color, {}, layer); }

void Renderer::flush() {
	if (!debug::is_production()) { return; }
	if (!m_current_texture) { return; }
	if (m_batch.is_empty()) { return; }
	m_target->draw(m_batch);
	++debug::draw_calls;
	m_batch.clear();
}

} // namespace fornani
