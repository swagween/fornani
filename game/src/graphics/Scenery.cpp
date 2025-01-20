
#include "fornani/graphics/Scenery.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Math.hpp"

namespace vfx {

Scenery::Scenery(automa::ServiceProvider& svc, sf::Vector2<float> position, int style, int layer, int variant, float parallax) : position(position), properties{style, layer, variant, parallax}, sprite{svc.assets.get_scenery(style)} {
	auto u = variant * dimensions.x;
	auto v = layer * dimensions.y;
	properties.parallax = 1.f - 0.1f / static_cast<float>((layer + 1));
	sprite.setTextureRect(sf::IntRect({u, v}, dimensions));
	sprite.setOrigin(f_dimensions * 0.5f);
}

void Scenery::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	sprite.setPosition(util::round_to_even(position) - cam * properties.parallax);
	win.draw(sprite);
}

} // namespace vfx
