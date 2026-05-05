
#include "fornani/graphics/Weather.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::vfx {

Weather::Weather(automa::ServiceProvider& svc, world::Map& map, WeatherParameters params) : m_parameters{params} {
	m_rect = sf::IntRect{{}, sf::Vector2i{map.real_dimensions} * 2};
	for (auto [l, layer] : std::views::enumerate(m_layers)) {
		layer.parallax = 1.f - l * 0.02f;
		for (auto [i, frame] : std::views::enumerate(layer.textures)) {
			auto sprite = sf::Sprite{svc.assets.get_texture("rain")};
			sprite.setTextureRect(sf::IntRect{{0, 256 * static_cast<int>(i)}, {256, 256}});

			auto u_dimensions = sf::Vector2u{256, 256};
			if (!frame.resize(u_dimensions)) { NANI_LOG_WARN(m_logger, "Failed to resize weather texture"); }
			frame.clear(sf::Color::Transparent);

			auto state = sf::RenderStates{sf::BlendNone};
			frame.draw(sprite, state);

			frame.display();
			frame.setRepeated(true);
		}
	}
}

void Weather::update(automa::ServiceProvider& svc, world::Map& map) {
	for (auto [l, layer] : std::views::enumerate(m_layers)) {
		auto local_framerate = m_parameters.framerate + l * 2;
		if (svc.ticker.every_x_ticks(local_framerate)) { layer.loop.modulate(1); }
	}
}

void Weather::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	for (auto [l, layer] : std::views::enumerate(m_layers)) {
		auto& frame = layer.textures[layer.loop.get()];
		for (auto i = 0; i < m_parameters.density; ++i) {
			auto sprite = sf::Sprite{frame.getTexture()};
			sprite.setTextureRect(m_rect);
			auto buffer = sf::Vector2f{-256.f, -256.f};
			layer.parallax = 1.1f;
			sprite.setPosition(buffer - cam * layer.parallax);
			sprite.setScale(constants::f_scale_vec);
			if (l == 0) { win.draw(sprite); }
		}
	}
}

} // namespace fornani::vfx
