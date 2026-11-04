
#include "fornani/graphics/Weather.hpp"
#include <fornani/core/Debug.hpp>
#include <numbers>
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::vfx {

Weather::Weather(automa::ServiceProvider& svc, world::Map& map, std::string_view tag) : m_parameters{svc.data.weather[tag]} {
	m_rect = sf::IntRect{{}, sf::Vector2i{map.real_dimensions} * 2};
	auto const tex = std::string{tag};
	svc.assets.get_texture_modifiable(tex).setRepeated(true);
	auto const p = m_parameters.parallax_factor;
	for (auto [l, layer] : std::views::enumerate(m_layers)) {
		layer.parallax = 1.f + (2.f * p) - static_cast<float>(l) * p;
		layer.seed = fornani::random::range(0.f, 2.f * std::numbers::pi_v<float>);
		for (auto [i, frame] : std::views::enumerate(layer.textures)) {
			auto sprite = sf::Sprite{svc.assets.get_texture(tex)};
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
		if (m_parameters.framerate > 0) {
			auto local_framerate = m_parameters.framerate + l * 8;
			if (svc.ticker.every_x_ticks(local_framerate)) { layer.loop.modulate(1); }
		} else {
			layer.position.x = 8.f * std::sin(svc.ticker.in_game_seconds_passed.count() + layer.seed);
			layer.position.y += 0.15f * layer.parallax;
			auto wrap = [](float value) { return std::fmod(std::fmod(value, 512.f) + 512.f, 512.f); };

			layer.position.y = wrap(layer.position.y);
		}
	}
}

void Weather::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam, std::size_t layer) {
	if (!debug::is_production()) { return; }
	auto& this_layer = m_layers[layer];
	auto& frame = this_layer.textures[this_layer.loop.get()];
	for (auto i = 0; i < m_parameters.density; ++i) {
		auto sprite = sf::Sprite{frame.getTexture()};
		sprite.setTextureRect(m_rect);
		auto buffer = sf::Vector2f{-180.f * layer + static_cast<float>(i) * 35.f, -512.f + static_cast<float>(i) * 65.f};
		sprite.setPosition((buffer - cam) * this_layer.parallax + this_layer.position);
		sprite.setScale(constants::f_scale_vec);
		win.draw(sprite);
		++debug::draw_calls;
	}
}

WeatherSpecifications::WeatherSpecifications(std::string_view type, float const chance) : type{type}, chance{chance} {}

WeatherSpecifications::WeatherSpecifications(dj::Json const& in) {
	type = in["type"].as_string();
	chance = in["chance"].as<float>();
}

void WeatherSpecifications::serialize(dj::Json& out) {
	out["type"] = type;
	out["chance"] = chance;
}

WeatherParameters::WeatherParameters(dj::Json const& in) {
	density = in["density"].as<int>();
	framerate = in["framerate"].as<int>();
	parallax_factor = in["parallax_factor"].as<float>();
	if (in["effect"]) { effect.emplace(in["effect"].as_string()); }
	if (in["ambience"]) { ambience.emplace(in["ambience"].as_string()); }
}

} // namespace fornani::vfx
