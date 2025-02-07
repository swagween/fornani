
#pragma once

namespace fornani {
namespace graphics {

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
	[[nodiscard]] auto is_shaking() const -> bool { return shake_properties.shaking; }
	[[nodiscard]] auto get_frequency() const -> int { return shake_properties.frequency; }
	[[nodiscard]] auto get_energy() const -> float { return shake_properties.energy; }
	[[nodiscard]] auto get_start_time() const -> int { return shake_properties.start_time; }
	[[nodiscard]] auto get_dampen_factor() const -> float { return shake_properties.dampen_factor; }

	[[nodiscard]] auto get_shake_properties() const -> ShakeProperties { return shake_properties; }

  private:
	ShakeProperties shake_properties{};
};

} // namespace graphics
} // namespace fornani
