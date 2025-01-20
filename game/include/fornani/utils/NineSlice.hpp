#pragma once
#include <array>
#include <SFML/Graphics.hpp>
#include "Cooldown.hpp"
#include "fornani/particle/Gravitator.hpp"

namespace automa {
struct ServiceProvider;
}

namespace util {

class NineSlice {
  public:
	NineSlice(automa::ServiceProvider& svc, int corner_factor, int edge_factor);
	void set_texture(sf::Texture& tex);
	void set_origin(sf::Vector2<float> origin);
	void update(automa::ServiceProvider& svc, sf::Vector2<float> position, sf::Vector2<float> dimensions, float corner_dim, float edge_dim);
	void render(sf::RenderWindow& win);
	void start(automa::ServiceProvider& svc, sf::Vector2<float> position);
	void end();
	void speed_up_appearance(int rate);
	void set_appearance_time(int time) { appearance_time = time; }
	void set_scale(float scale) { global_scale = scale; }
	void set_force(float force) { gravitator.attraction_force = force; }
	void set_fric(float fric) { gravitator.collider.physics.set_global_friction(fric); }
	void set_position(sf::Vector2<float> pos) { gravitator.set_position(pos); }
	[[nodiscard]] auto is_extended() const -> bool { return appear.is_complete(); }
	[[nodiscard]] auto get_center() const -> sf::Vector2<float> { return native_dimensions * 0.5f; }
	[[nodiscard]] auto get_position() const -> sf::Vector2<float> { return gravitator.collider.physics.position; }

  private:
	vfx::Gravitator gravitator{};
	sf::Vector2<float> random_offset{};
	int appearance_time{32};
	util::Cooldown appear{appearance_time};
	float global_scale{};
	sf::Sprite sprite;
	sf::Vector2<float> native_dimensions{};
	int corner_factor{};
	int edge_factor{};
	float corner_dimensions{};
	float edge_dimensions{};
};

} // namespace util