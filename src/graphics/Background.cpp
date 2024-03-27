
#include "Background.hpp"
#include "../service/ServiceProvider.hpp"
#include "../setup/EnumLookups.hpp"

namespace bg {

int const tile_dim{256};

Background::Background(automa::ServiceProvider& svc, int bg_id) {

	std::string_view type{};
	if (bg_id == 0) { type = "fast_scroll"; }
	else if (bg_id <= 4) { type = "standard_scroll"; }
	else if (bg_id >= 5) { type = "fast_scroll"; }
	else if (bg_id > 6) { type = "standard_flat"; }

	auto const& in_data = svc.data.background[type];
	behavior.used_layers = in_data["used_layers"].as<int>();
	behavior.scroll_speed = in_data["scroll_speed"].as<float>();
	behavior.scrolling = (bool)in_data["scrolling"].as_bool();
	behavior.parallax_multiplier = in_data["parallax_multiplier"].as<float>();

	int idx = 0;
	for (auto& sprite : sprites) {
		if (behavior.scrolling) {
			physics.push_back(components::PhysicsComponent());
			physics.back().air_friction = {1.f, 1.f};
			physics.back().ground_friction = {1.f, 1.f};
			sprite.setTextureRect(sf::IntRect({0, 540 * idx}, {3840, 540}));
		} else {
			sprite.setTextureRect(sf::IntRect({0, 0}, {tile_dim, tile_dim}));
		}
		sprite.setTexture(svc.assets.background_lookup.at(bg_id));
		++idx;
	}
}

void Background::update(automa::ServiceProvider& svc) {
	float frame_speedup = std::clamp(svc::cameraLocator.get().observed_velocity.x, 1.f, std::numeric_limits<float>::infinity()); // positive number
	frame_speedup *= 0.3f;

	if (behavior.scrolling) {
		if (physics.size() < sprites.size()) { return; }
		int idx = 0;
		for (auto& sprite : sprites) {
			physics.at(idx).velocity.x = -1 * (frame_speedup)*behavior.scroll_speed * idx; // negative
			physics.at(idx).update_euler(svc);

			if (physics.at(idx).position.x < -scroll_size) { physics.at(idx).position.x = 0.f; }

			sprite.setPosition(physics.at(idx).position.x, 0.f);
			++idx;
		}
	}
}

void Background::render(sf::RenderWindow& win, sf::Vector2<float>& campos, sf::Vector2<float>& mapdim) {
	if (behavior.scrolling) {
		for (auto& sprite : sprites) { win.draw(sprite); }
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
