
#pragma once

#include <SFML/Graphics.hpp>
#include <cstdint>

namespace fornani::graphics {

enum class CameraState { constrained, free };
enum class CameraOwner { player, system };

struct ShakeProperties {
	bool shaking{};
	int frequency{};
	float energy{};
	int start_time{};
	int dampen_factor{};
};

class CameraController {
  public:
	void shake(ShakeProperties properties);
	void shake(int frequency = 10, float energy = 0.4f, int start_time = 400, int dampen_factor = 30);
	void cancel();
	void free();
	void constrain();
	void set_owner(CameraOwner to);
	void set_position(sf::Vector2f to) { m_position = to; }

	[[nodiscard]] auto is_shaking() const -> bool { return shake_properties.shaking; }
	[[nodiscard]] auto get_frequency() const -> int { return shake_properties.frequency; }
	[[nodiscard]] auto get_energy() const -> float { return shake_properties.energy; }
	[[nodiscard]] auto get_start_time() const -> int { return shake_properties.start_time; }
	[[nodiscard]] auto get_dampen_factor() const -> float { return shake_properties.dampen_factor; }
	[[nodiscard]] auto get_shake_properties() const -> ShakeProperties { return shake_properties; }

	[[nodiscard]] auto get_state() const -> CameraState { return m_state; }
	[[nodiscard]] auto get_position() const -> sf::Vector2f { return m_position; }
	[[nodiscard]] auto is_free() const -> bool { return m_state == CameraState::free; }
	[[nodiscard]] auto is_owned_by(CameraOwner test) const -> bool { return m_owner == test; }

  private:
	ShakeProperties shake_properties{};
	CameraState m_state{};
	CameraOwner m_owner{};
	sf::Vector2f m_position{};
};

} // namespace fornani::graphics
