
#include "Background.hpp"
#include "../setup/EnumLookups.hpp"

namespace bg {

int const tile_dim{256};

Background::Background(BackgroundBehavior b, int bg_id) : behavior(b) {
	if (b.scrolling) {
		int idx = 0;
		for (auto& sprite : sprites) {
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
}

void Background::update() {
	if (behavior.scrolling) {
		int idx = 0;
		for (auto& sprite : sprites) {
			int offset = idx * (-frames[idx] * behavior.scroll_speed);
			if (offset < -1920) { frames[idx] = 0; }
			sprite.setPosition(idx * (-frames[idx] * behavior.scroll_speed), 0);
			++idx;
		}
		for (auto& frame : frames) { ++frame; }
	}
}

void Background::render(sf::RenderWindow& win, sf::Vector2<float>& campos, sf::Vector2<float>& mapdim) {
	if (behavior.scrolling) {
		for (auto& sprite : sprites) {
			win.draw(sprite);
			svc::counterLocator.get().at(svc::draw_calls)++;
		}
	} else if (!sprites.empty()) {
		for (int i = 0; i < mapdim.x / behavior.parallax_multiplier; i += tile_dim) {
			for (int j = 0; j < mapdim.y / behavior.parallax_multiplier; j += tile_dim) {
				sprites.at(0).setPosition(i - behavior.parallax_multiplier * campos.x, j - behavior.parallax_multiplier * campos.y);
				win.draw(sprites.at(0));
				svc::counterLocator.get().at(svc::draw_calls)++;
			}
		}
	}
}

} // namespace bg
