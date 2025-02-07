
#include "fornani/graphics/CameraController.hpp"

namespace fornani {

namespace graphics {

void CameraController::shake(ShakeProperties properties) { shake_properties = properties; }

void CameraController::shake(int frequency, float energy, int start_time, int dampen_factor) { shake_properties = {true, frequency, energy, start_time, dampen_factor}; }

void CameraController::cancel() { shake_properties = {}; }

} // namespace graphics

} // namespace fornani
