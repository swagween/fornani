
#include <fornani/particle/Emitter.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Random.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::vfx {

Emitter::Emitter(automa::ServiceProvider& svc, world::Map& map, sf::Vector2f position, sf::Vector2f dimensions, std::string_view type, sf::Color color, Direction direction)
	: position(position), dimensions(dimensions), type(type), color(color), direction(direction) {
	auto const& in_data = svc.data.particle[type];
	variables.load = in_data["load"].as<int>();
	variables.rate = in_data["rate"].as<float>();
	particle_dimensions.x = in_data["dimensions"][0].as<float>();
	particle_dimensions.y = in_data["dimensions"][1].as<float>();

	m_load = util::Cooldown(variables.load);
	m_load.start();
	drawbox.setFillColor(sf::Color::Transparent);
	drawbox.setOutlineThickness(-1);
	drawbox.setOutlineColor(sf::Color::Red);
	drawbox.setSize(dimensions);

	auto x = random::random_range_float(-dimensions.x * 0.5f, dimensions.x * 0.5f);
	auto y = random::random_range_float(-dimensions.y * 0.5f, dimensions.y * 0.5f);
	sf::Vector2f point{position.x + x, position.y + y};
	particles.push_back(std::make_unique<Particle>(svc, map, point, particle_dimensions, type, color, direction));
}

void Emitter::update(automa::ServiceProvider& svc, world::Map& map) {
	m_load.update();
	std::erase_if(particles, [](auto const& p) { return p->done(); });
	if (m_load.running() && (random::percent_chance(variables.rate) || particles.empty())) {
		auto x = random::random_range_float(0.f, dimensions.x);
		auto y = random::random_range_float(0.f, dimensions.y);
		sf::Vector2f point{position.x + x, position.y + y};
		particles.push_back(std::make_unique<Particle>(svc, map, point, particle_dimensions, type, color, direction));
	}
	for (auto& particle : particles) { particle->update(svc, map); }
}

void Emitter::render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam) {
	if (svc.greyblock_mode()) {
		drawbox.setPosition(position - cam - drawbox.getLocalBounds().size * 0.5f);
		win.draw(drawbox);
	}
	for (auto& particle : particles) { particle->render(svc, win, cam); }
}

void Emitter::set_position(sf::Vector2f pos) { position = pos; }

void Emitter::set_dimensions(sf::Vector2f dim) { dimensions = dim; }

} // namespace fornani::vfx
