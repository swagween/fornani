
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/gizmos/RotaryGizmo.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Math.hpp>
#include <fornani/world/Map.hpp>
#include <cmath>

namespace fornani::gui {

RotaryGizmo::RotaryGizmo(automa::ServiceProvider& svc, world::Map& map, player::Player& player, sf::Vector2f placement)
	: Gizmo("Rotary", false), m_path{svc.finder, std::filesystem::path{"/data/gui/gizmo_paths.json"}, "rotary", 96, util::InterpolationType::quadratic}, m_selection{static_cast<int>(player.arsenal_size())},
	  m_sprite{svc, "rotary_gizmo", {124, 94}}, m_frame_lerp{6.f} {
	m_foreground = true;
	m_dashboard_port = DashboardPort::arsenal;
	m_placement = placement;
	m_path.set_section("start");
	m_previous_position = m_frame_lerp.raw();
}

void RotaryGizmo::update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map, sf::Vector2f position) {
	Gizmo::update(svc, player, map, position);
	if (m_state == GizmoState::selected && m_switched) {
		on_open(svc, player, map);
	} else if (m_switched) {
		on_close(svc, player, map);
	}
	if (is_closed() && m_exit_trigger) {
		m_path.set_section("end");
		m_exit_trigger = false;
	}
	m_sprite.tick();
	m_frame_lerp.update(svc.ticker.dt.count());
	m_path.update();

	float current_pos = m_frame_lerp.raw();
	auto range = m_frame_lerp.get_range() * 0.5f;
	int prev_tick = static_cast<int>(std::floor(m_previous_position / range));
	int curr_tick = static_cast<int>(std::floor(current_pos / range));

	if (prev_tick != curr_tick) { svc.soundboard.play_sound("pioneer_rotary", player.get_ear_position()); }

	m_previous_position = current_pos;
}

void RotaryGizmo::render(automa::ServiceProvider& svc, sf::RenderWindow& win, [[maybe_unused]] player::Player& player, LightShader& shader, Palette& palette, sf::Vector2f cam, bool foreground) {
	Gizmo::render(svc, win, player, shader, palette, cam, foreground);
	if (is_foreground() != foreground) { return; }

	m_sprite.set_channel(1);
	auto frame = static_cast<int>(m_frame_lerp.get());
	m_sprite.set_frame(frame);
	m_sprite.set_position(m_placement + m_path.get_position() - cam);
	shader.submit(win, palette, m_sprite.get_sprite());

	m_sprite.set_channel(0);
	m_sprite.set_frame(frame);
	m_sprite.set_position(m_placement + m_path.get_position() - cam);
	shader.submit(win, palette, m_sprite.get_sprite());

	debug();
}

bool RotaryGizmo::handle_inputs(input::InputSystem& controller, [[maybe_unused]] audio::Soundboard& soundboard) { return Gizmo::handle_inputs(controller, soundboard); }

void RotaryGizmo::on_open(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map) {
	Gizmo::on_open(svc, player, map);
	m_frame_lerp.trigger(4, false, 16.0f);
}

void RotaryGizmo::on_close(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map) {
	Gizmo::on_close(svc, player, map);
	m_frame_lerp.trigger(4, true, 16.0f);
}

void RotaryGizmo::debug() {
	static auto sz = ImVec2{180.f, 450.f};
	ImGui::SetNextWindowSize(sz);
	if (ImGui::Begin("Rotary Debug")) {
		ImGui::SeparatorText("Controls");
		if (ImGui::Button("spin forwards")) { m_frame_lerp.trigger(8, true, 8.f); }
		if (ImGui::Button("spin backwards")) { m_frame_lerp.trigger(2, false, 8.0f); }
		ImGui::Text("Selection: %i", m_selection.get());
		ImGui::Text("Lerp: %.3f", m_frame_lerp.get());
		ImGui::End();
	}
}

} // namespace fornani::gui
