#include "fornani/graphics/SpriteHistory.hpp"

#include <ccmath/ext/clamp.hpp>
#include <ccmath/math/misc/lerp.hpp>

namespace fornani::graphics {

SpriteHistory::SpriteHistory(int sample_size) : m_sample_size(sample_size) {}

void SpriteHistory::update(sf::Sprite next, sf::Vector2f position) {
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
		pair.first.setColor(sf::Color(255, 255, 255, a));
		pair.first.setPosition(pair.second - cam);
		win.draw(pair.first);
		a = ccm::lerp(m_dimness_limit, 255, range);
		a = ccm::ext::clamp(a, 0, 255);
		range += 1.f / static_cast<float>(m_pairs.size());
	}
}

} // namespace fornani::graphics
