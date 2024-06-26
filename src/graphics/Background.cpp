
#include "Background.hpp"
#include "../service/ServiceProvider.hpp"
#include "../setup/EnumLookups.hpp"
#include <algorithm>

namespace bg {

int const tile_dim{256};

Background::Background(automa::ServiceProvider& svc, int bg_id) {

	std::string_view type{};
	if (bg_id == 0) { type = "fast_scroll"; }
	else if (bg_id <= 4) { type = "standard_scroll"; }
	else if (bg_id >= 5) { type = "fast_scroll"; }
	if (bg_id > 6) { type = "standard_flat"; }
	if (bg_id == 17) { type = "slow_static"; }

	auto const& in_data = svc.data.background[type];
	behavior.used_layers = in_data["used_layers"].as<int>();
	behavior.scroll_speed = in_data["scroll_speed"].as<float>();
	behavior.scrolling = (bool)in_data["scrolling"].as_bool();
	behavior.parallax_multiplier = in_data["parallax_multiplier"].as<float>();
	dimensions.x = in_data["dimensions"][0].as<int>();
	dimensions.y = in_data["dimensions"][1].as<int>();
	start_offset.x = in_data["start_offset"][0].as<int>();
	start_offset.y = in_data["start_offset"][1].as<int>();

	for (auto i{0}; i < behavior.used_layers; ++i) { sprites.push_back(sf::Sprite()); }

	int idx = 0;
	for (auto& sprite : sprites) {
		if (behavior.scrolling) {
			physics.push_back(components::PhysicsComponent());
			physics.back().air_friction = {1.f, 1.f};
			physics.back().ground_friction = {1.f, 1.f};
			sprite.setTextureRect(sf::IntRect({0, dimensions.y * idx}, dimensions));
		} else {
			sprite.setTextureRect(sf::IntRect({0, dimensions.y * idx}, dimensions));
		}
		sprite.setTexture(svc.assets.background_lookup.at(bg_id));
		++idx;
	}
}

void Background::update(automa::ServiceProvider& svc, sf::Vector2<float> observed_camvel) {
	float frame_speedup = observed_camvel.x; // bugged
	frame_speedup = 1.f;

	if (behavior.scrolling) {
		if (physics.size() < sprites.size()) { return; }
		int idx = 0;
		for (auto& sprite : sprites) {
			physics.at(idx).velocity.x = -frame_speedup * behavior.scroll_speed * idx; // negative
			physics.at(idx).update_euler(svc);

			if (physics.at(idx).position.x < -scroll_size) { physics.at(idx).position.x = 0.f; }

			sprite.setPosition(physics.at(idx).position.x - static_cast<float>(start_offset.x), static_cast<float>(-start_offset.y));
			++idx;
		}
	}
}

void Background::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float>& campos, sf::Vector2<float>& mapdim) {
	if (behavior.scrolling) {
		auto idx{0};
		for (auto& sprite : sprites) {
			sf::Vector2<float> final_position = physics.at(idx).position - campos * behavior.parallax_multiplier * (float)idx - sf::Vector2<float>{start_offset};
			if(dimensions.y == svc.constants.screen_dimensions.y) {}
			final_position.y = std::clamp(final_position.y, (float)svc.constants.screen_dimensions.y - (float)dimensions.y, 0.01f);
			sprite.setPosition(final_position);
			win.draw(sprite);
			++idx;
		}
	} else if (!sprites.empty()) {
		for (int i = 0; i < mapdim.x / behavior.parallax_multiplier; i += tile_dim) {
			for (int j = 0; j < mapdim.y / behavior.parallax_multiplier; j += tile_dim) {
				sprites.at(0).setPosition(i - behavior.parallax_multiplier * campos.x - start_offset.x, j - behavior.parallax_multiplier * campos.y - start_offset.y);
				win.draw(sprites.at(0));
			}
		}
	}
}

} // namespace bg
