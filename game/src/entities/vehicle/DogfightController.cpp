
#include <fornani/entities/vehicle/DogfightController.hpp>
#include <fornani/systems/InputSystem.hpp>

namespace fornani {

void DogfightController::update(automa::ServiceProvider& svc) {
	m_steering.thrust_seek(m_target, {0.2f + 0.2f * m_dash.get_normalized(), .918f, .998f, 60.f});
	m_dash.update();
}

void DogfightController::handle_input(input::InputSystem& input) {
	auto const left = input.direction_held(input::AnalogAction::move, input::MoveDirection::left);
	auto const right = input.direction_held(input::AnalogAction::move, input::MoveDirection::right);
	auto const up = input.direction_held(input::AnalogAction::move, input::MoveDirection::up);
	auto const down = input.direction_held(input::AnalogAction::move, input::MoveDirection::down);

	sf::Vector2f direction{static_cast<float>(right) - static_cast<float>(left), static_cast<float>(down) - static_cast<float>(up)};

	auto const length = std::hypot(direction.x, direction.y);
	constexpr float buffer = 32.f;
	auto const skip = m_dash.just_started() ? 180.f : 0.f;
	auto const speed = m_speed + sf::Vector2f{0.8f, 0.8f} * skip;

	if (length > 0.f) {
		direction /= length;
		m_target.x = std::clamp(m_target.x + direction.x * speed.x, buffer, 960.f - buffer);
		m_target.y = std::clamp(m_target.y + direction.y * speed.y, buffer, 512.f - buffer);
	}

	if (input.digital(input::DigitalAction::jump).triggered && m_dash.is_complete()) { m_dash.start(); }
}

} // namespace fornani
