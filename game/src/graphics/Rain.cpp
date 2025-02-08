#include "fornani/graphics/Rain.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::vfx {

Rain::Rain(int intensity, float fall_speed, float slant, bool snow, bool collision) : intensity(intensity), fall_speed(fall_speed), slant(slant), collision(collision) {
	auto color = sf::Color{163, 206, 229, 100};
	auto snow_color = sf::Color{206, 236, 239, 200};
	snow ? raindrop.setFillColor(snow_color) : raindrop.setFillColor(color);
	variance = 2.f;
	dimensions = {3.f, 16.f};
	if (snow) { dimensions = {4.f, 4.f};
		sway = 0.2f;
	}
}

void Rain::update(automa::ServiceProvider& svc, world::Map& map) {
	float offset = -slant * 32.f;
	if (svc.ticker.every_x_ticks(4)) {
		for (int i{0}; i < intensity; ++i) {
			z = svc.random.random_range_float(0.999f, 1.f);
			//fall_speed *= z;
			auto start_x = svc.random.random_range_float(offset, map.real_dimensions.x + offset);
			auto start_y = svc.random.random_range_float(-64.f, 0.f);
			auto tweak = svc.random.random_range_float(-variance, variance);
			sf::Vector2<float> start{start_x, start_y};
			drops.push_back(Droplet{start, {slant, sway, fall_speed}, tweak});
			drops.back().z = z;
		}
	}
	for (auto& drop : drops) { drop.update(svc, map, collision); }
	std::erase_if(drops, [](auto const& d) { return d.decayed(); });
}

void Rain::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	for (auto& drop : drops) {
		drop.collided() ? raindrop.setSize({dimensions.x, dimensions.x}) : raindrop.setSize(dimensions);
		drop.collided() ? raindrop.setOrigin({dimensions.x * 0.5f, dimensions.x}) : raindrop.setOrigin({dimensions.x * 0.5f, dimensions.y});
		raindrop.setPosition(drop.position() - cam);
		raindrop.setRotation(sf::degrees(drop.get_angle()));
		raindrop.setScale({drop.z, raindrop.getScale().y});
		svc.greyblock_mode() ? drop.collider.render(win, cam) : win.draw(raindrop);
	}
}

} // namespace vfx
