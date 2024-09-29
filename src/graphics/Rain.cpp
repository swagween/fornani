#include "Rain.hpp"
#include "../service/ServiceProvider.hpp"

namespace vfx {
Rain::Rain(int intensity, float fall_speed, float slant, bool snow) : intensity(intensity), fall_speed(fall_speed), slant(slant) {
	auto color = sf::Color{163, 206, 229, 100};
	auto snow_color = sf::Color{206, 236, 239, 200};
	snow ? raindrop.setFillColor(color) : raindrop.setFillColor(snow_color);
	variance = 2.f;
	dimensions = {3.f, 16.f};
	if (snow) { dimensions = {4.f, 4.f};
		sway = 0.2f;
	}
}
void Rain::update(automa::ServiceProvider& svc, world::Map& map) {
	float offset = -slant * 32.f;
	if (svc.ticker.every_x_ticks(10)) {
		for (int i{0}; i < intensity; ++i) {
			auto start_x = svc.random.random_range_float(offset, svc.constants.f_screen_dimensions.x + offset);
			auto start_y = svc.random.random_range_float(-64.f, 0.f);
			auto tweak = svc.random.random_range_float(-variance, variance);
			sf::Vector2<float> start{start_x, start_y};
			drops.push_back(Droplet{start, {slant, sway, fall_speed}, tweak});
		}
	}
	for (auto& drop : drops) { drop.update(svc, map); }
	std::erase_if(drops, [](auto const& d) { return d.decayed(); });
}

void Rain::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	for (auto& drop : drops) {
		drop.collided() ? raindrop.setSize({dimensions.x, dimensions.x}) : raindrop.setSize(dimensions);
		drop.collided() ? raindrop.setOrigin({dimensions.x * 0.5f, dimensions.x}) : raindrop.setOrigin({dimensions.x * 0.5f, dimensions.y});
		raindrop.setPosition(drop.position() - cam);
		raindrop.setRotation(drop.get_angle());
		svc.greyblock_mode() ? drop.collider.render(win, cam) : win.draw(raindrop);
	}
}

} // namespace vfx
