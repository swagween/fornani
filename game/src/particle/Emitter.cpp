#include "fornani/particle/Emitter.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Random.hpp"

namespace fornani::vfx {

Emitter::Emitter(automa::ServiceProvider& svc, sf::Vector2f position, sf::Vector2f dimensions, std::string_view type, sf::Color color, Direction direction)
	: position(position), dimensions(dimensions), type(type), color(color), direction(direction) {
	auto const& in_data = svc.data.particle[type];
	variables.load = in_data["load"].as<int>();
	variables.rate = in_data["rate"].as<float>();
	particle_dimensions.x = in_data["dimensions"][0].as<float>();
	particle_dimensions.y = in_data["dimensions"][1].as<float>();

	cooldown = util::Cooldown(variables.load);
	cooldown.start();
	drawbox.setFillColor(sf::Color::Transparent);
	drawbox.setOutlineThickness(-1);
	drawbox.setOutlineColor(sf::Color::Red);
	drawbox.setSize(dimensions);
}

void Emitter::update(automa::ServiceProvider& svc, world::Map& map) {
	cooldown.update();
	if (cooldown.is_complete()) { deactivate(); }
	if (active && (util::random::percent_chance(variables.rate) || particles.empty())) {
		auto x = util::random::random_range_float(0.f, dimensions.x);
		auto y = util::random::random_range_float(0.f, dimensions.y);
		sf::Vector2f point{position.x + x, position.y + y};
		particles.push_back(Particle(svc, point, particle_dimensions, type, color, direction));
	}
	for (auto& particle : particles) { particle.update(svc, map); }
	std::erase_if(particles, [](auto const& p) { return p.done(); });
}

void Emitter::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	if (svc.greyblock_mode()) {
		drawbox.setPosition(position - cam);
		win.draw(drawbox);
	}
	for (auto& particle : particles) { particle.render(svc, win, cam); }
}

void Emitter::set_position(sf::Vector2f pos) { position = pos; }

void Emitter::set_dimensions(sf::Vector2f dim) { dimensions = dim; }

void Emitter::deactivate() { active = false; }

} // namespace fornani::vfx