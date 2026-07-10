
#include "fornani/graphics/Weather.hpp"
#include <fornani/core/Debug.hpp>
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::vfx {

Weather::Weather(automa::ServiceProvider& svc, world::Map& map, WeatherParameters params) : m_parameters{params} {
	m_rect = sf::IntRect{{}, sf::Vector2i{map.real_dimensions} * 2};
	for (auto [l, layer] : std::views::enumerate(m_layers)) {
		layer.parallax = 1.1f - l * 0.05f;
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
		auto local_framerate = m_parameters.framerate + l * 8;
		if (svc.ticker.every_x_ticks(local_framerate)) { layer.loop.modulate(1); }
	}
}

void Weather::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam, std::size_t layer) {
	auto& this_layer = m_layers[layer];
	auto& frame = this_layer.textures[this_layer.loop.get()];
	for (auto i = 0; i < m_parameters.density; ++i) {
		auto sprite = sf::Sprite{frame.getTexture()};
		sprite.setTextureRect(m_rect);
		auto buffer = sf::Vector2f{-180.f * layer + static_cast<float>(i) * 35.f, -256.f + static_cast<float>(i) * 65.f};
		sprite.setPosition(util::round_to_even((buffer - cam) * this_layer.parallax));
		sprite.setScale(constants::f_scale_vec);
		win.draw(sprite);
		++debug::draw_calls;
	}
}

WeatherSpecifications::WeatherSpecifications() {
	type = "rain";
	effect = "raindrop";
	ambience = "rain";
	chance = 1.f;
	params = {1, 8};
}

WeatherSpecifications::WeatherSpecifications(dj::Json const& in) {
	type = in["type"].as_string();
	effect = in["effect"].as_string();
	ambience = in["ambience"].as_string();
	chance = in["chance"].as<float>();
	params = {in["density"].as<int>(), in["framerate"].as<int>()};
}

void WeatherSpecifications::serialize(dj::Json& out) {
	out["type"] = type;
	out["effect"] = effect;
	out["ambience"] = ambience;
	out["chance"] = chance;
	out["framerate"] = params.framerate;
	out["density"] = params.density;
}

} // namespace fornani::vfx
