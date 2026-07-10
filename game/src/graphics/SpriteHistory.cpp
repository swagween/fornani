
#include <fornani/core/Debug.hpp>
#include <fornani/graphics/Renderer.hpp>
#include <fornani/graphics/SpriteHistory.hpp>
#include <algorithm>
#include <cmath>

namespace fornani::graphics {

SpriteHistory::SpriteHistory(int sample_size) : m_sample_size(sample_size) {}

void SpriteHistory::update(sf::Sprite next, sf::Vector2f position) {
	next.setScale({1.f, 1.f});
	m_pairs.push_back({next, position});
	if (m_pairs.size() >= m_sample_size) { m_pairs.pop_front(); }
}

void SpriteHistory::flush() {
	if (m_pairs.size() > 0) { m_pairs.pop_front(); }
}

void SpriteHistory::drag(sf::RenderWindow& win, sf::Vector2f cam) {
	auto a{m_dimness_limit};
	auto range{0.f};
	for (auto& pair : m_pairs) {
		pair.first.setColor(sf::Color(180, 180, 255, a));
		pair.first.setPosition(pair.second - cam);
		win.draw(pair.first);
		++debug::draw_calls;
		a = std::lerp(m_dimness_limit, 255, range);
		a = std::clamp(a, 0, 255);
		range += 1.f / static_cast<float>(m_pairs.size());
	}
}

void SpriteHistory::submit(Renderer& renderer) {
	float alpha = static_cast<float>(m_dimness_limit);
	float range = 0.f;
	for (auto& pair : m_pairs) {
		auto& sprite = pair.first;
		sf::Color color{180, 180, 255, static_cast<std::uint8_t>(alpha)};
		sf::Vector2f pos = pair.second;

		auto const bounds = sprite.getGlobalBounds();
		auto const uv = sprite.getTextureRect();

		sf::FloatRect dest{sf::Vector2f{std::floor(pos.x), std::floor(pos.y)}, bounds.size};
		renderer.submit(sprite.getTexture(), dest, uv, constants::f_scale_factor, color, {}, RenderLayer::atmosphere);
		alpha = std::lerp(static_cast<float>(m_dimness_limit), 255.f, range);
		alpha = std::clamp(alpha, 0.f, 255.f);
		range += 1.f / static_cast<float>(m_pairs.size());
	}
}

} // namespace fornani::graphics
