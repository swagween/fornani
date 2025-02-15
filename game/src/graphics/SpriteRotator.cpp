#include "fornani/graphics/SpriteRotator.hpp"
#include "fornani/utils/Math.hpp"
#include <numbers>

namespace fornani::vfx {

void SpriteRotator::handle_rotation(sf::Sprite& sprite, sf::Vector2<float> direction, int num_angles, bool radial) {

	sprite.setScale({1.f, 1.f});
	sprite.setRotation(sf::degrees(0.f));

	sprite_angle_index = 0;
	auto angle = util::direction(direction);

	auto pi = static_cast<float>(std::numbers::pi);
	auto two_pi = pi * 2.f;
	auto three_halves_pi = 3.f * pi / 2.f;
	auto three_quarters_pi = 3.f * pi / 4.f;
	auto half_pi = pi / 2.f;
	auto one_quarter_pi = pi / 4.f;

	auto modulus = std::fmodf(angle + pi, half_pi);
	for (int i{num_angles - 1}; i >= 0; --i) {
		if (modulus <= ((2 * i + 1) * pi) / (num_angles * 4.f)) { sprite_angle_index = i; }
	}

	auto slice = (pi / (num_angles * 4.f));
	auto positive_angle = angle + pi + slice;

	if (radial) {
		if (positive_angle <= two_pi && positive_angle > three_halves_pi) { sprite.setRotation(sf::degrees(90.f));
		}
		if (positive_angle <= three_halves_pi + slice && positive_angle > pi) {}
		if (positive_angle <= pi && positive_angle > half_pi) { sprite.setRotation(sf::degrees(270.f));
		}
		if (positive_angle <= half_pi || positive_angle > two_pi) { sprite.setRotation(sf::degrees(180.f));
		}
	} else {
		if (positive_angle <= two_pi && positive_angle > three_halves_pi) { sprite.setScale({-1.f, 1.f}); }
		if (positive_angle <= three_halves_pi + slice && positive_angle > pi) {}
		if (positive_angle <= pi && positive_angle > half_pi) {}
		if (positive_angle <= half_pi || positive_angle > two_pi) { sprite.setScale({-1.f, 1.f}); }
	}
}

} // namespace vfx
