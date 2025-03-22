
#include "fornani/graphics/CameraController.hpp"

namespace fornani::graphics {

void CameraController::shake(ShakeProperties properties) { shake_properties = properties; }

void CameraController::shake(int frequency, float energy, int start_time, int dampen_factor) { shake_properties = {true, frequency, energy, start_time, dampen_factor}; }

void CameraController::cancel() { shake_properties = {}; }

void CameraController::free() { m_state = CameraState::free; }

void CameraController::constrain() { m_state = CameraState::constrained; }

} // namespace fornani::graphics
