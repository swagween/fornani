
#pragma once

#include <SFML/Graphics.hpp>

namespace fornani::vfx {
class SpriteRotator {
  public:
	void handle_rotation(sf::Sprite& sprite, sf::Vector2<float> direction, int num_angles, bool radial = true);
	[[nodiscard]] auto get_sprite_angle_index() const -> int { return sprite_angle_index; }

  private:
	int sprite_angle_index{};
};

} // namespace vfx
