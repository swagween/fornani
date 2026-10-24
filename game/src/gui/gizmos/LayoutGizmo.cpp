
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/gizmos/LayoutGizmo.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/systems/InputSystem.hpp>

namespace fornani::gui {

LayoutGizmo::LayoutGizmo(automa::ServiceProvider& svc, world::Map& map, sf::Vector2f placement)
	: Gizmo("Layout", false), m_board{svc, svc.assets.get_texture("layout_board"), {38, 38}, {1, 1}}, m_path{svc.finder, std::filesystem::path{"/data/gui/gizmo_paths.json"}, "layout", 160, util::InterpolationType::quadratic},
	  m_slots{svc, "slot_fixtures", {28, 28}} {
	m_path.set_section("open");
	m_slots.push_and_set_animation("basic", {0, 1, 32, -1});
	m_slots.center();
	m_placement = placement;
}

void LayoutGizmo::update(automa::ServiceProvider& svc, player::Player& player, world::Map& map, sf::Vector2f position) {
	Gizmo::update(svc, player, map, position);
	m_slots.tick();
	m_physics.position = get_placement() + position;
	m_path.update();
	m_board.set_position(m_path.get_position() + get_placement());
	m_board.set_dimensions(m_path.get_dimensions());
}

void LayoutGizmo::render(automa::ServiceProvider& svc, sf::RenderWindow& win, player::Player& player, LightShader& shader, Palette& palette, sf::Vector2f cam, bool foreground) {
	if (is_foreground() != foreground) { return; }
	auto const base = m_board.get_global_center() - cam;
	m_board.render(win, cam);
	if (m_current_layout != nullptr) {
		auto const half = 18.f;
		auto const offset = half * arms::layout_size_v;
		for (auto [i, slot] : std::views::enumerate(m_current_layout->get_slots())) {
			auto const y = i % 2 == 0 ? -half : half;
			m_slots.set_position(base + sf::Vector2f{static_cast<float>(i) * half * 2.f - offset, y});
			m_slots.set_channel(std::min(2, static_cast<int>(slot.status)));
			if (slot.index && has_flag_set(LayoutGizmoFlags::selected)) {
				if (m_selection.get() == static_cast<int>(*slot.index)) { m_slots.set_channel(2); }
			}
			if (slot.pin_locations.empty()) { win.draw(m_slots); }
			for (auto const& pin : slot.pin_locations) {
				m_slots.set_frame(static_cast<int>(pin));
				win.draw(m_slots);
			}
		}
	}
	debug();
}

bool LayoutGizmo::handle_inputs(input::InputSystem& controller, audio::Soundboard& soundboard) {
	if (controller.digital(input::DigitalAction::menu_select).triggered) {}
	if (controller.digital(input::DigitalAction::menu_back).triggered) {
		controller.flush_triggers();
		return true;
	}
	if (controller.menu_move(input::MoveDirection::left)) { m_selection.modulate(-1); }
	if (controller.menu_move(input::MoveDirection::right)) {
		m_selection.modulate(1);
		// if (m_selection.cycled()) { return true; }
	}
	return false;
}

void LayoutGizmo::set_layout(arms::Weapon& weapon) {
	m_current_layout = &weapon.get_layout();
	m_selection.set_order(m_current_layout->get_number_of_slots());
}

void LayoutGizmo::debug() {
	static auto sz = ImVec2{380.f, 450.f};
	ImGui::SetNextWindowSize(sz);
	if (ImGui::Begin("Layout Debug")) {
		ImGui::Text("Selection: %i", m_selection.get());
		ImGui::Text("Order: %i", m_selection.get_order());
		ImGui::Text("Slots: %i", m_current_layout->get_number_of_slots());
		ImGui::Text("List: (");
		for (auto const& slot : m_current_layout->get_slots()) {
			ImGui::SameLine();
			if (slot.index) { ImGui::Text("##%i,", *slot.index); }
		}
		ImGui::SameLine();
		ImGui::Text(")");
		ImGui::Text("Path position: (%.1f, %.1f)", m_path.get_position().x, m_path.get_position().y);
		ImGui::Text("Layout position: (%.1f, %.1f)", m_board.get_global_center().x, m_board.get_global_center().y);
		ImGui::End();
	}
}

} // namespace fornani::gui
