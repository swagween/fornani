
#pragma once

#include <SFML/Graphics.hpp>
#include <string_view>
#include <unordered_map>
#include "Animation.hpp"
#include "fornani/graphics/SpriteRotator.hpp"

namespace fornani::automa {
struct ServiceProvider;
} // namespace fornani::automa

namespace fornani::anim {

class AnimatedSprite {
  public:
	// ReSharper disable once CppNonExplicitConvertingConstructor
	AnimatedSprite(sf::Texture const& texture, sf::Vector2<int> dimensions = {32, 32});
	void update(sf::Vector2<float> pos, int u = 0, int v = 0, bool horiz = false);
	void push_params(std::string_view label, Parameters in_params);
	void set_params(std::string_view label, bool force = false);
	void set_dimensions(sf::Vector2<int> dim);
	void set_position(sf::Vector2<float> pos);
	void set_scale(sf::Vector2<float> scale);
	void set_origin(sf::Vector2<float> origin);
	void set_texture(sf::Texture const& texture);
	void random_start();
	void handle_rotation(sf::Vector2<float> direction, int num_angles, bool radial = true);
	void end();
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam, bool debug = false);
	[[nodiscard]] auto get_frame() const -> int { return animation.get_frame(); }
	[[nodiscard]] auto just_started() const -> bool { return animation.just_started(); }
	[[nodiscard]] auto complete() -> bool { return animation.complete(); }
	[[nodiscard]] auto size() const -> int { return static_cast<int>(params.size()); }
	[[nodiscard]] auto get_sprite_angle_index() const -> int { return rotator.get_sprite_angle_index(); }
	[[nodiscard]] auto get_position() const -> sf::Vector2<float> { return position; }
	[[nodiscard]] auto get_sprite_position() const -> sf::Vector2<float> { return sprite.getPosition(); }
	[[nodiscard]] auto get_dimensions() const -> sf::Vector2<float> { return sprite.getGlobalBounds().size; }
	Animation& get() { return animation; }
	sf::Sprite& get_sprite() { return sprite; }

  private:
	sf::Sprite sprite;
	sf::RectangleShape drawbox{};
	sf::Vector2<int> dimensions{};
	sf::Vector2<float> position{};
	Animation animation{};
	std::unordered_map<std::string_view, Parameters> params{};
	vfx::SpriteRotator rotator{};
};

} // namespace fornani::anim
