#include "Sparkler.hpp"
#include "Sparkler.hpp"
#include "Sparkler.hpp"
#include "../setup/ServiceLocator.hpp"

namespace vfx {

void Sparkler::update() { 

	if (svc::randomLocator.get().percent_chance(2)) {
		auto x = svc::randomLocator.get().random_range_float(0.f, dimensions.x);
		auto y = svc::randomLocator.get().random_range_float(0.f, dimensions.y);
		sf::Vector2<float> point{position.x + x, position.y + y};
		sparkles.push_back(Spark(point, color));
	}
	for (auto& spark : sparkles) {
		spark.update();
		std::erase_if(sparkles, [](auto const& s) { return s.done(); });
	}
}

void Sparkler::render(sf::RenderWindow& win, sf::Vector2<float> cam) {

	for (auto& spark : sparkles) { spark.render(win, cam); }
}

void Sparkler::set_position(sf::Vector2<float> pos) { position = pos; }

void Sparkler::set_dimensions(sf::Vector2<float> dim) { dimensions = dim; }

} // namespace vfx