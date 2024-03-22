#include "Emitter.hpp"
#include "../setup/ServiceLocator.hpp"
#include "../service/ServiceProvider.hpp"

namespace vfx {

Emitter::Emitter(sf::Vector2<float> position, sf::Vector2<float> dimensions, std::string_view type, sf::Color color, dir::Direction direction) : position(position), dimensions(dimensions), type(type), color(color), direction(direction) {
	cooldown.start(18);
	drawbox.setFillColor(sf::Color::Transparent);
	drawbox.setOutlineThickness(-1);
	drawbox.setOutlineColor(sf::Color::Red);
	drawbox.setSize(dimensions);
}

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
		particle.update(svc, map);
		std::erase_if(particles, [](auto const& p) { return p.done(); });
	}
}

void Emitter::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam) {
	if (svc.greyblock_mode()) {
		drawbox.setPosition(position - cam);
		win.draw(drawbox);
	}
	for (auto& particle : particles) { particle.render(svc, win, cam); }
}

void Emitter::set_position(sf::Vector2<float> pos) { position = pos; }

void Emitter::set_dimensions(sf::Vector2<float> dim) { dimensions = dim; }

void Emitter::deactivate() { active = false; }

} // namespace vfx