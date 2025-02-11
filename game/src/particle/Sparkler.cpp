#include "fornani/particle/Sparkler.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Random.hpp"

namespace fornani::vfx {

Sparkler::Sparkler(automa::ServiceProvider& svc, sf::Vector2<float> dimensions, sf::Color color, std::string_view type) : dimensions(dimensions), color(color), type(type) {
	auto const& in_data = svc.data.sparkler[type];
	behavior.rate = in_data["rate"].as<float>();
	drawbox.setFillColor(sf::Color::Transparent);
	drawbox.setOutlineThickness(-1);
	drawbox.setOutlineColor(sf::Color::Red);
	drawbox.setSize(dimensions);
}

void Sparkler::update(automa::ServiceProvider& svc) {
	if (util::Random::percent_chance(behavior.rate)) {
		auto const x = util::Random::random_range_float(0.f, dimensions.x);
		auto const y = util::Random::random_range_float(0.f, dimensions.y);
		sf::Vector2 const point{position.x + x, position.y + y};
		if (active) { sparkles.push_back(Spark(svc, point, color, type)); }
	}
	for (auto& spark : sparkles) { spark.update(svc); }
	std::erase_if(sparkles, [](auto const& s) { return s.done(); });
}

void Sparkler::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	if (svc.greyblock_mode()) {
		drawbox.setPosition(position - cam);
		drawbox.setSize(dimensions);
		win.draw(drawbox);
	} else {
		for (auto& spark : sparkles) { spark.render(win, cam); }
	}
}

void Sparkler::set_position(sf::Vector2<float> pos) { position = pos; }

void Sparkler::set_dimensions(sf::Vector2<float> dim) { dimensions = dim; }

void Sparkler::activate() { active = true; }

void Sparkler::deactivate() { active = false; }

void Sparkler::set_color(sf::Color to_color) { color = to_color; }

void Sparkler::set_rate(float to_rate) { behavior.rate = to_rate; }

} // namespace fornani::vfx