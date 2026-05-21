
#include <fornani/core/Debug.hpp>
#include <fornani/graphics/Renderer.hpp>
#include <fornani/utils/Constants.hpp>

namespace fornani {

void Renderer::begin(sf::RenderWindow& win, sf::Vector2f cam) {
	m_target = &win;
	m_batch.clear();
	m_batch.setPosition(-cam);
	m_current_texture = nullptr;
}

void Renderer::submit(sf::Texture const& texture, sf::FloatRect dest, sf::IntRect uv, sf::Color color) {
	if (&texture != m_current_texture) {
		flush();
		m_current_texture = &texture;
		m_batch.setTexture(texture);
	}
	dest.size *= constants::f_scale_factor;
	m_batch.add(dest, uv, color);
}

void Renderer::flush() {
	if (!m_current_texture) { return; }
	if (m_batch.is_empty()) { return; }
	m_target->draw(m_batch);
	++debug::draw_calls;
	m_batch.clear();
}

} // namespace fornani
