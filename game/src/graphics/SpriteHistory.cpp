#include "fornani/graphics/SpriteHistory.hpp"

#include <ccmath/ext/clamp.hpp>
#include <ccmath/math/misc/lerp.hpp>

namespace fornani::flfx {

void SpriteHistory::update(sf::Sprite next, sf::Vector2<float> position) {
	pairs.push_back({next, position});
	if (pairs.size() >= sample_size) { pairs.pop_front(); }
}

void SpriteHistory::flush() {
	if (pairs.size() > 0) { pairs.pop_front(); }
}

void SpriteHistory::drag(sf::RenderWindow& win, sf::Vector2<float> cam) {
	auto a{dimness_limit};
	auto range{0.f};
	for (auto& pair : pairs) {
		pair.first.setColor(sf::Color(255, 255, 255, a));
		pair.first.setPosition(pair.second - cam);
		win.draw(pair.first);
		a = ccm::lerp(dimness_limit, 255, range);
		a = ccm::ext::clamp(a, 0, 255);
		range += 1.f / static_cast<float>(pairs.size());
	}
}

} // namespace fornani::flfx
