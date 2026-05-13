
#pragma once

#include <fornani/particle/Particle.hpp>
#include <fornani/utils/Flaggable.hpp>
#include <vector>

namespace fornani::vfx {

enum class EmitterFlags { map };

struct EmitterParameters {
	std::string tag{};
	int frequency{};
	sf::Vector2f dimensions{16.f, 16.f};
	sf::Vector2f offset{};
	Direction direction{UND::up};
};

class Emitter : public Flaggable<EmitterFlags> {
  public:
	Emitter() = default;
	Emitter(automa::ServiceProvider& svc, sf::Vector2f position, sf::Vector2f dimensions, std::string_view type, sf::Color color = sf::Color::Transparent, Direction direction = {}, int channel = 0);
	Emitter(automa::ServiceProvider& svc, world::Map& map, sf::Vector2f position, sf::Vector2f dimensions, std::string_view type, sf::Color color = sf::Color::Transparent, Direction direction = {}, int channel = 0);
	void init(automa::ServiceProvider& svc, sf::Vector2f position, sf::Vector2f dimensions, std::string_view type, sf::Color color = sf::Color::Transparent, Direction direction = {});

	void update(automa::ServiceProvider& svc, world::Map& map);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f cam);
	void render(sf::RenderWindow& win, sf::Vector2f cam);
	void set_position(sf::Vector2f pos);
	void set_dimensions(sf::Vector2f dim);
	[[nodiscard]] auto done() const -> bool { return particles.empty(); }

  private:
	std::vector<std::unique_ptr<Particle>> particles{};
	sf::Vector2f dimensions{};
	sf::Vector2f particle_dimensions{3.f, 3.f}; // customize later
	sf::Vector2f position{};

	struct {
		int load{};
		float rate{};
	} variables{};

	std::string type{};
	sf::Color color{};
	sf::RectangleShape drawbox{}; // for debug
	util::Cooldown m_load{};
	Direction direction{};
};

} // namespace fornani::vfx
