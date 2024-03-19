#include "Emitter.hpp"
#include "../setup/ServiceLocator.hpp"

namespace vfx {

Emitter::Emitter(sf::Vector2<float> position, sf::Vector2<float> dimensions, std::string_view type, sf::Color color, dir::Direction direction) : position(position), dimensions(dimensions), type(type), color(color), direction(direction) { cooldown.start(8); }

void Emitter::update(automa::ServiceProvider& svc, world::Map& map) {
	cooldown.update();
	if (cooldown.is_complete()) { deactivate(); }
	if (active) {
		auto x = svc::randomLocator.get().random_range_float(0.f, dimensions.x);
		auto y = svc::randomLocator.get().random_range_float(0.f, dimensions.y);
		sf::Vector2<float> point{position.x + x, position.y + y};
		 particles.push_back(Particle(svc, point, particle_dimensions, type, color, direction));
	
	}
	for (auto& particle : particles) {
		particle.update(map);
		std::erase_if(particles, [](auto const& p) { return p.done(); });
	}
}

void Emitter::render(sf::RenderWindow& win, sf::Vector2<float> cam) {
	for (auto& particle : particles) { particle.render(win, cam); }
}

void Emitter::set_position(sf::Vector2<float> pos) { position = pos; }

void Emitter::set_dimensions(sf::Vector2<float> dim) { dimensions = dim; }

void Emitter::deactivate() { active = false; }

} // namespace vfx