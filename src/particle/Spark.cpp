#include "Spark.hpp"
#include "../service/ServiceProvider.hpp"
#include <numbers>
#include <ccmath/math/power/sqrt.hpp>

namespace vfx {

Spark::Spark(automa::ServiceProvider& svc, sf::Vector2<float> pos, sf::Color color, std::string_view type) : type(type) {
	auto const& in_data = svc.data.sparkler[type];
	parameters.wobble = in_data["wobble"].as<float>();
	parameters.frequency = in_data["frequency"].as<float>();
	parameters.speed = in_data["speed"].as<float>();
	position = pos;
	box.setFillColor(color);
	box.setSize({3.f, 3.f});
	auto variance = in_data["lifespan_variance"].as<int>();
	auto rand_diff = svc.random.random_range(-variance, variance);
	lifespan.start(in_data["lifespan"].as<int>() + rand_diff);
	parameters.volatility = in_data["volatility"].as<float>();

	//seed variables
	variables.energy = svc.random.random_range_float(1.0f - parameters.volatility, 1.0f + parameters.volatility);
	variables.offset = svc.random.random_range_float(0.f, static_cast<float>(std::numbers::pi) * 2.f);

}

void Spark::update(automa::ServiceProvider& svc) {
	position.x += variables.energy * parameters.wobble * sin(parameters.frequency * frame + variables.offset);
	position.y -= variables.energy * parameters.speed;
	++frame;
	lifespan.update();
}

void Spark::render(sf::RenderWindow& win, sf::Vector2<float> cam) {
	box.setPosition(position - cam);
	win.draw(box);
}

} // namespace vfx