#pragma once
#include <array>
#include <SFML/Graphics.hpp>
#include "Cooldown.hpp"
#include "../particle/Gravitator.hpp"

namespace automa {
struct ServiceProvider;
}

namespace util {

class NineSlice {
  public:
	void slice(int corner_factor, int edge_factor);
	void set_texture(sf::Texture& tex);
	void set_origin(sf::Vector2<float> origin);
	void update(automa::ServiceProvider& svc, sf::Vector2<float> position, sf::Vector2<float> dimensions, float corner_dim, float edge_dim);
	void render(sf::RenderWindow& win) const;
	void start(automa::ServiceProvider& svc, sf::Vector2<float> position);
	void end();
	void set_scale(float scale) { global_scale = scale; }
	[[nodiscard]] auto is_extended() const -> bool { return appear.is_complete(); }
	[[nodiscard]] auto get_center() const -> sf::Vector2<float> { return native_dimensions * 0.5f; }
	[[nodiscard]] auto get_position() const -> sf::Vector2<float> { return gravitator.collider.physics.position; }

  private:
	vfx::Gravitator gravitator{};
	sf::Vector2<float> random_offset{};
	int appearance_time{32};
	util::Cooldown appear{appearance_time};
	float global_scale{};
	std::array<sf::Sprite, 9> sprites{};
	sf::Vector2<float> native_dimensions{};
};

} // namespace util