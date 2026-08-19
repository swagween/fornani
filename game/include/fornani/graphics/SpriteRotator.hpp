
#pragma once

#include <SFML/Graphics.hpp>

class Animatable;

namespace fornani::vfx {
enum class AnimatableAxis : std::uint8_t { frame, channel };
class SpriteRotator {
  public:
	void handle_rotation(Animatable& sprite, sf::Vector2f direction, AnimatableAxis axis, int num_angles, bool radial = true);
	void handle_rotation(sf::Sprite& sprite, sf::Vector2f direction, int num_angles, bool radial = true);
	[[nodiscard]] auto get_sprite_angle_index() const -> int { return sprite_angle_index; }

  private:
	int sprite_angle_index{};
};

} // namespace fornani::vfx
