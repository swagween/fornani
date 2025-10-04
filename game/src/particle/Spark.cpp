
#include <ccmath/math/power/sqrt.hpp>
#include <fornani/particle/Spark.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <numbers>

namespace fornani::vfx {

Spark::Spark(automa::ServiceProvider& svc, sf::Vector2f pos, sf::Color color, std::string_view type) : type(type) {
	auto const& in_data = svc.data.sparkler[type];
	parameters.wobble = in_data["wobble"].as<float>();
	parameters.frequency = in_data["frequency"].as<float>();
	parameters.speed = in_data["speed"].as<float>();
	position = pos;
	box.setFillColor(color);
	box.setSize({3.f, 3.f});
	auto const variance = in_data["lifespan_variance"].as<int>();
	auto const rand_diff = random::random_range(-variance, variance);
	lifespan.start(in_data["lifespan"].as<int>() + rand_diff);
	parameters.volatility = in_data["volatility"].as<float>();

	// seed variables
	variables.energy = random::random_range_float(1.0f - parameters.volatility, 1.0f + parameters.volatility);
	variables.offset = random::random_range_float(0.f, static_cast<float>(std::numbers::pi) * 2.f);

	if (in_data["fader"].as_bool()) { fader = util::Fader(svc, lifespan.get(), in_data["color"].as_string()); }
	if (fader) { fader.value().get_sprite().setScale({3.f, 3.f}); }

	if (in_data["animation"].is_object()) {
		auto const& in_anim = in_data["animation"];
		m_sprite = Animatable(svc, "sparkler_" + in_anim["label"].as_string(), {in_anim["dimensions"][0].as<int>(), in_anim["dimensions"][1].as<int>()});
		m_sprite->set_parameters({0, in_anim["num_frames"].as<int>(), in_anim["framerate"].as<int>(), 0});
		m_sprite->set_channel(random::random_range(0, in_anim["variants"].as<int>()));
		m_sprite->center();
	}
}

void Spark::update(automa::ServiceProvider& svc) {
	if (m_sprite) { m_sprite->tick(); }
	position.x += variables.energy * parameters.wobble * sin(parameters.frequency * frame + variables.offset);
	position.y -= variables.energy * parameters.speed;
	++frame;
	lifespan.update();
	if (fader) { fader.value().update(); }
}

void Spark::render(sf::RenderWindow& win, sf::Vector2f cam) {
	box.setPosition(position - cam);
	if (fader) {
		fader.value().get_sprite().setPosition(position - cam);
		win.draw(fader.value().get_sprite());
	} else if (m_sprite) {
		m_sprite->set_position(position - cam);
		win.draw(*m_sprite);
	} else {
		win.draw(box);
	}
}

} // namespace fornani::vfx
