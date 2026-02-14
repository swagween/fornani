
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/gizmos/RotaryGizmo.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Math.hpp>
#include <fornani/world/Map.hpp>
#include <cmath>

namespace fornani::gui {

RotaryGizmo::RotaryGizmo(automa::ServiceProvider& svc, world::Map& map, player::Player& player, sf::Vector2f placement)
	: Gizmo("Rotary", false), m_path{svc.finder, std::filesystem::path{"/data/gui/gizmo_paths.json"}, "rotary", 120, util::InterpolationType::quadratic}, m_selection{static_cast<int>(player.arsenal_size())},
	  m_sprite{svc, "rotary_gizmo", {124, 94}}, m_gun_selector{svc, "gun_selector", {64, 31}}, m_frame_lerp{6.f}, m_gun_display{svc, "heads_up_display_gun", {44, 9}}, m_dashboard_rail{svc, "pioneer_dashboard"},
	  m_rail_path{svc.finder, std::filesystem::path{"/data/gui/gizmo_paths.json"}, "dashboard_rail", 256, util::InterpolationType::quadratic} {
	m_dashboard_port = DashboardPort::arsenal;
	m_placement = placement;
	m_path.set_section("start");
	m_previous_position = m_frame_lerp.raw();
	m_dashboard_rail.set_texture_rect({{657, 0}, {175, 174}});
	m_gun_display.set_origin({0.f, 4.5f});
	m_gun_selector.push_and_set_animation("standard", {0, 10, 24, -1});
	m_gun_selector.center();
}

void RotaryGizmo::update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map, sf::Vector2f position) {
	Gizmo::update(svc, player, map, position);
	m_physics.position = m_path.get_position() + position;
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
	m_gun_selector.tick();
	m_frame_lerp.update(svc.ticker.dt.count());
	m_path.update();
	m_rail_path.update();

	if (m_hotbar) { m_hotbar.value()->update(svc, player, map, m_placement + m_rail_path.get_position()); }

	if (player.arsenal) {
		if (player.arsenal->get_loadout().size() >= m_selection.get_order()) {
			m_gun_id = player.arsenal->get_loadout().at(m_selection.get())->get_id();
			if (consume_flag(RotaryGizmoFlags::push_to_hotbar)) {
				if (auto tag = svc.data.get_gun_tag_from_id(m_gun_id)) {
					auto popped = false;
					if (player.hotbar) {
						popped = player.hotbar->has(*tag);
						popped ? player.remove_from_hotbar(*tag) : player.add_to_hotbar(*tag);
					} else {
						player.add_to_hotbar(*tag);
					}
					popped ? svc.soundboard.flags.console.set(audio::Console::done) : svc.soundboard.flags.pioneer.set(audio::Pioneer::slot);
					auto key = popped ? "popped_from_hotbar" : "pushed_to_hotbar";
					auto qualifier = svc.data.gui_text["notifications"][key].as_string();
					auto message = qualifier + std::string{*tag} + ".";
					svc.notifications.push_notification(svc, message);
				}
			}
		}
	}

	float current_pos = m_frame_lerp.raw() + 0.5f;
	auto range = m_frame_lerp.get_range() * 0.5f;
	int prev_tick = static_cast<int>(std::floor(m_previous_position / range));
	int curr_tick = static_cast<int>(std::floor(current_pos / range));
	static bool alternated = false;
	if (prev_tick != curr_tick) {
		alternated ? svc.soundboard.play_sound("pioneer_rotary", player.get_ear_position()) : svc.soundboard.play_sound("pioneer_tick", player.get_ear_position());
		alternated = !alternated;
	}

	m_previous_position = current_pos;
}

void RotaryGizmo::render(automa::ServiceProvider& svc, sf::RenderWindow& win, [[maybe_unused]] player::Player& player, LightShader& shader, Palette& palette, sf::Vector2f cam, bool foreground) {
	Gizmo::render(svc, win, player, shader, palette, cam, foreground);
	if (is_foreground() != foreground) { return; }

	if (is_selected()) {
		m_dashboard_rail.set_position(m_placement + m_rail_path.get_position() + m_rail_path.get_dimensions() - cam);
		win.draw(m_dashboard_rail);
	}

	m_sprite.set_channel(1);
	auto frame = static_cast<int>(m_frame_lerp.get());
	m_sprite.set_frame(frame);
	m_sprite.set_position(m_placement + m_path.get_position() - cam);
	shader.submit(win, palette, m_sprite.get_sprite());

	if (m_hotbar) {
		m_hotbar.value()->render(svc, win, player, shader, palette, cam);
		m_hotbar.value()->set_hovered_gun(m_gun_id);
	}

	if (player.arsenal) {
		for (auto i = -1; i <= 1; ++i) {
			auto selection = m_selection.get(i);
			auto gun_id = player.arsenal->get_loadout().at(selection)->get_id();
			auto gun_offset = sf::Vector2f{48.f, 96.f};
			if (auto tag = svc.data.get_gun_tag_from_id(gun_id)) {
				m_gun_display.set_channel(3);
				if (player.hotbar) {
					if (player.hotbar->has(*tag)) { m_gun_display.set_channel(i == 0 ? 2 : 3); }
				}
				auto xoff = i == 0 ? 0.f : -16.f;
				m_gun_display.set_position(m_placement + m_path.get_position() + gun_offset + sf::Vector2f{xoff, static_cast<float>(i) * 56.f} - cam);
				m_gun_display.set_frame(gun_id);
				win.draw(m_gun_display);
			}
		}
		auto selector_offset = sf::Vector2f{100.f, 96.f};
		m_gun_selector.set_position(m_placement + m_path.get_position() + selector_offset - cam);
		if (is_selected()) { win.draw(m_gun_selector); }
	}

	m_sprite.set_channel(0);
	m_sprite.set_frame(frame);
	m_sprite.set_position(m_placement + m_path.get_position() - cam);
	shader.submit(win, palette, m_sprite.get_sprite());
}

bool RotaryGizmo::handle_inputs(input::InputSystem& controller, [[maybe_unused]] audio::Soundboard& soundboard) {
	if (controller.digital(input::DigitalAction::menu_select).triggered) { set_flag(RotaryGizmoFlags::push_to_hotbar); }
	if (controller.menu_move(input::MoveDirection::up, input::DigitalActionQueryType::triggered)) {
		m_frame_lerp.trigger(1, false, 2.0f);
		m_selection.modulate(-1);
	}
	if (controller.menu_move(input::MoveDirection::down, input::DigitalActionQueryType::triggered)) {
		m_frame_lerp.trigger(1, true, 2.0f);
		m_selection.modulate(1);
	}
	return Gizmo::handle_inputs(controller, soundboard);
}

void RotaryGizmo::on_open(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map) {
	Gizmo::on_open(svc, player, map);
	m_frame_lerp.trigger(6, false, 4.0f);
	m_path.set_section("open");
	m_rail_path.set_section("open");
	m_hotbar = std::make_unique<HotbarGizmo>(svc, map, sf::Vector2f{200.f, 0.f});
	svc.soundboard.play_sound("pioneer_tech", player.get_ear_position());
}

void RotaryGizmo::on_close(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map) {
	Gizmo::on_close(svc, player, map);
	m_frame_lerp.trigger(4, true, 8.0f);
	m_path.set_section("close");
	m_rail_path.set_section("close");
	m_hotbar.reset();
	svc.soundboard.play_sound("pioneer_drawer", player.get_ear_position());
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
