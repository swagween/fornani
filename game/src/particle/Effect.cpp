#include "fornani/particle/Effect.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Random.hpp"

namespace fornani::entity {

Effect::Effect(automa::ServiceProvider& svc, std::string const& label, sf::Vector2f pos, sf::Vector2f vel, int channel)
	: Animatable(svc, "effect_" + label, {svc.data.effect[label]["dimensions"][0].as<int>(), svc.data.effect[label]["dimensions"][1].as<int>()}) {
	set_channel(channel);
	if (util::random::percent_chance(50)) {
		if (svc.data.effect[label]["x_reflection"].as_bool()) { scale({-1.f, 1.f}); }
		if (svc.data.effect[label]["y_reflection"].as_bool()) { scale({1.f, -1.f}); }
	}
	center();
	set_parameters({0, svc.data.effect[label]["frame_count"].as<int>(), svc.data.effect[label]["framerate"].as<int>(), 0});
	physics = components::PhysicsComponent({0.99f, 0.99f}, 1.f);
	physics.position = pos;
	physics.velocity = vel;
}

void Effect::update() {
	Animatable::tick();
	physics.simple_update();
}

void Effect::render(sf::RenderWindow& win, sf::Vector2f cam) {
	set_position(physics.position - cam);
	win.draw(*this);
}

void Effect::rotate() { Drawable::rotate(sf::degrees(90)); }

} // namespace fornani::entity
