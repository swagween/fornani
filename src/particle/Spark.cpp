#include "Spark.hpp"
#include "../service/ServiceProvider.hpp"
#include <numbers>

namespace vfx {

Spark::Spark(automa::ServiceProvider& svc, sf::Vector2<float> pos, sf::Color color, std::string_view type) : type(type) {
	auto const& in_data = svc.data.sparkler[type];
	parameters.wobble = in_data["wobble"].as<float>();
	parameters.frequency = in_data["frequency"].as<float>();
	parameters.speed = in_data["speed"].as<int>();
	position = pos;
	box.setFillColor(color);
	box.setSize({3.f, 3.f});
	int rand_diff = svc.random.random_range(0, 50);
	lifespan.start(200 + rand_diff);
}

void Spark::update(automa::ServiceProvider& svc) {
	++frame;
	auto offset = svc.random.random_range_float(0, std::numbers::pi * 2.f * parameters.frequency);
	position.x += parameters.wobble * sin(parameters.frequency * frame + offset);
	if (svc.ticker.every_x_frames(parameters.speed)) { --position.y; }
		
	lifespan.update();
}

void Spark::render(sf::RenderWindow& win, sf::Vector2<float> cam) { 
	
	box.setPosition(position - cam);
	win.draw(box);
}

bool Spark::done() const { return lifespan.is_complete(); }


} // namespace vfx