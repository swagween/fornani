
#pragma once

#include <SFML/Graphics.hpp>
#include "fornani/graphics/CameraController.hpp"
#include "fornani/particle/Gravitator.hpp"
#include "fornani/utils/BitFlags.hpp"
#include "fornani/utils/Cooldown.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani {

constexpr float CAM_FRICTION = 0.89f;
constexpr float CAM_MASS = 1.0f;
constexpr float CAM_GRAV = 0.09f;

constexpr int CX_OFFSET = 60;
constexpr int CY_OFFSET = 60;

constexpr int border_buffer{32};

enum class CamFlags : uint8_t { shake };

class Camera {
  public:
	Camera();
	void update(automa::ServiceProvider& svc);
	void restrict_movement(sf::Vector2<float>& bounds);
	void fix_horizontally(sf::Vector2<float> map_dim);
	void fix_vertically(sf::Vector2<float> map_dim);
	void set_position(sf::Vector2<float> new_pos);
	void center(sf::Vector2<float> new_position);
	void force_center(sf::Vector2<float> new_position);
	void begin_shake();

	[[nodiscard]] auto get_observed_velocity() const -> sf::Vector2<float> { return observed_velocity; }
	[[nodiscard]] auto get_position() const -> sf::Vector2<float> { return display_position; }
	[[nodiscard]] auto within_frame(int x, int y) const -> bool { return (x > 0) && (x < screen_dimensions.x + border_buffer) && (y > 0) && (y < screen_dimensions.y + border_buffer); }

  private:
	vfx::Gravitator gravitator{};
	sf::Vector2<int> screen_dimensions{};
	sf::RectangleShape bounding_box{};
	sf::Vector2<float> display_position{};
	sf::Vector2<float> observed_velocity{};
	sf::Vector2<float> previous_position{};
	sf::Vector2<float> target{};
	sf::Vector2<float> map_bounds_offset{};
	util::BitFlags<CamFlags> flags{};
	float grav_force{CAM_GRAV};

	struct {
		util::Cooldown timer{};
		util::Cooldown dampen{};
		graphics::ShakeProperties properties{};
	} shake{};
};

} // namespace fornani
