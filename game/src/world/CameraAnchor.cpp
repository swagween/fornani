
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/world/CameraAnchor.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

CameraAnchor::CameraAnchor(sf::Vector2f position) : m_point{position} {}

void CameraAnchor::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) {
	auto const focus = player.get_camera_focus_point();
	auto const distance = focus - m_point;
	auto const length = distance.length();

	constexpr float influence_radius = 400.f;

	if (length >= influence_radius) { return; }
	auto const t = length / influence_radius;
	auto const weight = 1.f - t * t * (3.f - 2.f * t);

	svc.camera_controller.add_anchor(m_point, weight);
}

} // namespace fornani
