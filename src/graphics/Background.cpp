
#include "Background.hpp"
#include "../setup/EnumLookups.hpp"

namespace bg {

int const tile_dim{256};

Background::Background(BackgroundBehavior b, int bg_id, services::ServiceLocator& svc) : behavior(b) {
	if (b.scrolling) {
		int idx = 0;
		for (auto& sprite : sprites) {
			physics.push_back(components::PhysicsComponent());
			physics.back().air_friction = {1.f, 1.f};
			physics.back().ground_friction = {1.f, 1.f};
			sprite.setTextureRect(sf::IntRect({0, 540 * idx}, {3840, 540}));
			sprite.setTexture(get_backdrop_texture.at(bg_id));
			++idx;
		}
	} else {
		int idx = 0;
		for (auto& sprite : sprites) {
			sprite.setTextureRect(sf::IntRect({0, 0}, {tile_dim, tile_dim}));
			sprite.setTexture(get_backdrop_texture.at(bg_id));
			++idx;
		}
	}
	get_backdrop_texture.insert({0, svc.assetLocator.get().t_bg_dusk});
	get_backdrop_texture.insert({1, svc.assetLocator.get().t_bg_sunrise});
	get_backdrop_texture.insert({2, svc.assetLocator.get().t_bg_opensky});
	get_backdrop_texture.insert({3, svc.assetLocator.get().t_bg_rosyhaze});
	get_backdrop_texture.insert({4, svc.assetLocator.get().t_bg_dawn});
	get_backdrop_texture.insert({5, svc.assetLocator.get().t_bg_night});
	get_backdrop_texture.insert({6, svc.assetLocator.get().t_bg_overcast});
	get_backdrop_texture.insert({7, svc.assetLocator.get().t_bg_slime});
	get_backdrop_texture.insert({8, svc.assetLocator.get().t_bg_black});
	get_backdrop_texture.insert({9, svc.assetLocator.get().t_bg_navy});
	get_backdrop_texture.insert({10, svc.assetLocator.get().t_bg_dirt});
	get_backdrop_texture.insert({11, svc.assetLocator.get().t_bg_gear});
	get_backdrop_texture.insert({12, svc.assetLocator.get().t_bg_library});
	get_backdrop_texture.insert({13, svc.assetLocator.get().t_bg_granite});
	get_backdrop_texture.insert({14, svc.assetLocator.get().t_bg_ruins});
	get_backdrop_texture.insert({15, svc.assetLocator.get().t_bg_crevasse});
	get_backdrop_texture.insert({16, svc.assetLocator.get().t_bg_deep});
	get_backdrop_texture.insert({17, svc.assetLocator.get().t_bg_grove});
}

void Background::update(cam::Camera& camera, services::ServiceLocator& svc) {
	float frame_speedup = std::clamp(camera.observed_velocity.x, 1.f, std::numeric_limits<float>::infinity()); // positive number

	if (behavior.scrolling) {
		if (physics.size() < sprites.size()) { return; }
		int idx = 0;
		for (auto& sprite : sprites) {
			physics.at(idx).velocity.x = -1 * (frame_speedup)*behavior.scroll_speed * idx; // negative
			physics.at(idx).update(svc.tickerLocator.get().tick_rate);

			if (physics.at(idx).position.x < -scroll_size) { physics.at(idx).position.x = 0.f; }

			sprite.setPosition(physics.at(idx).position.x, 0.f);
			++idx;
		}
	}
}

void Background::render(sf::RenderWindow& win, sf::Vector2<float>& campos, sf::Vector2<float>& mapdim) {
	if (behavior.scrolling) {
		for (auto& sprite : sprites) {
			win.draw(sprite);
		}
	} else if (!sprites.empty()) {
		for (int i = 0; i < mapdim.x / behavior.parallax_multiplier; i += tile_dim) {
			for (int j = 0; j < mapdim.y / behavior.parallax_multiplier; j += tile_dim) {
				sprites.at(0).setPosition(i - behavior.parallax_multiplier * campos.x, j - behavior.parallax_multiplier * campos.y);
				win.draw(sprites.at(0));
			}
		}
	}
}

} // namespace bg
