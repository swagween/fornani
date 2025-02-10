#include "fornani/particle/Spark.hpp"
#include <ccmath/math/power/sqrt.hpp>
#include <numbers>
#include "fornani/service/ServiceProvider.hpp"

#include "fornani/utils/Random.hpp"

namespace fornani::vfx {

Spark::Spark(automa::ServiceProvider& svc, sf::Vector2<float> pos, sf::Color color, std::string_view type) : type(type) {
	auto const& in_data = svc.data.sparkler[type];
	parameters.wobble = in_data["wobble"].as<float>();
	parameters.frequency = in_data["frequency"].as<float>();
	parameters.speed = in_data["speed"].as<float>();
	position = pos;
	box.setFillColor(color);
	box.setSize({3.f, 3.f});
	auto const variance = in_data["lifespan_variance"].as<int>();
	auto const rand_diff = util::Random::random_range(-variance, variance);
	lifespan.start(in_data["lifespan"].as<int>() + rand_diff);
	parameters.volatility = in_data["volatility"].as<float>();

	// seed variables
	variables.energy = util::Random::random_range_float(1.0f - parameters.volatility, 1.0f + parameters.volatility);
	variables.offset = util::Random::random_range_float(0.f, static_cast<float>(std::numbers::pi) * 2.f);

	if (in_data["fader"].as_bool()) { fader = util::Fader(svc, lifespan.get_cooldown(), in_data["color"].as_string()); }
	if (fader) { fader.value().get_sprite().setScale({3.f, 3.f}); }
}

void Spark::update(automa::ServiceProvider& svc) {
	position.x += variables.energy * parameters.wobble * sin(parameters.frequency * frame + variables.offset);
	position.y -= variables.energy * parameters.speed;
	++frame;
	lifespan.update();
	if (fader) { fader.value().update(); }
}

void Spark::render(sf::RenderWindow& win, sf::Vector2<float> cam) {
	box.setPosition(position - cam);
	if (fader) {
		fader.value().get_sprite().setPosition(position - cam);
		win.draw(fader.value().get_sprite());
	} else {
		win.draw(box);
	}
}

} // namespace fornani::vfx