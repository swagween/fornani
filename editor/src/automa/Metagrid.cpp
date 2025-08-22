
#include <imgui.h>
#include <editor/automa/Metagrid.hpp>
#include <imgui-SFML.h>

namespace pi {

Metagrid::Metagrid(fornani::automa::ServiceProvider& svc) : EditorState(svc), m_workspace{{64, 64}} {
	p_target_state = EditorStateType::metagrid;
	p_wallpaper.setFillColor(m_background_color);
	p_wallpaper.setSize(p_services->window->f_screen_dimensions());
	for (auto& map : svc.data.map_jsons) { m_rooms.push_back(Room(map.metadata)); }
}

EditorStateType Metagrid::run() {
	logic();

	EditorState::render(p_services->window->get());
	render(p_services->window->get());
	ImGui::SFML::Render(p_services->window->get());
	p_services->window->get().display();

	return p_target_state;
}

void Metagrid::handle_events(std::optional<sf::Event> event, sf::RenderWindow& win) {
	ImGuiIO& io = ImGui::GetIO();
	m_current_mouse_position = sf::Vector2f{io.MousePos.x, io.MousePos.y};
	if (auto const* button_pressed = event->getIf<sf::Event::MouseButtonPressed>()) {
		if (button_pressed->button == sf::Mouse::Button::Middle) { pressed_keys.set(PressedKeys::mouse_middle); }
		if (button_pressed->button == sf::Mouse::Button::Left) { pressed_keys.set(PressedKeys::mouse_left); }
		if (button_pressed->button == sf::Mouse::Button::Right) { pressed_keys.set(PressedKeys::mouse_right); }
	}
	if (auto const* button_released = event->getIf<sf::Event::MouseButtonReleased>()) {
		if (button_released->button == sf::Mouse::Button::Middle) { pressed_keys.reset(PressedKeys::mouse_middle); }
		if (button_released->button == sf::Mouse::Button::Left) { pressed_keys.reset(PressedKeys::mouse_left); }
		if (button_released->button == sf::Mouse::Button::Right) { pressed_keys.reset(PressedKeys::mouse_right); }
	}
}

void Metagrid::logic() {
	if (pressed_keys.test(PressedKeys::mouse_right)) { m_camera += m_current_mouse_position - m_right_clicked_position; }
	m_right_clicked_position = sf::Vector2f{m_current_mouse_position};
	for (auto& r : m_rooms) { r.update(m_current_mouse_position); }
}

void Metagrid::render(sf::RenderWindow& win) {
	m_workspace.render(win, m_camera);

	bool menu_open = ImGui::IsAnyItemHovered();

	// render rooms
	auto found_one{false};
	for (auto& r : m_rooms) {
		if (!r.is_highlighted()) {
			r.render(win, m_camera);
		} else {
			found_one = true;
			if (!menu_open) { m_highlighted_room = std::make_unique<Room>(r); }
		}
	}
	if (!found_one && !menu_open) { m_highlighted_room = {}; }
	if (m_highlighted_room) { m_highlighted_room.value()->render(win, m_camera); }

	// ImGui stuff

	bool options_popup{pressed_keys.consume(PressedKeys::mouse_middle) && m_highlighted_room};
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) { ImGui::EndMenu(); }
		if (ImGui::Button("Editor")) { p_target_state = EditorStateType::editor; }
		ImGui::EndMainMenuBar();
	}

	if (options_popup) { ImGui::OpenPopup("Room Options"); }
	if (ImGui::BeginPopupContextWindow("Room Options")) {
		if (ImGui::Button("Minimap")) {
			if (m_highlighted_room) { m_highlighted_room.value()->toggle_minimap(); }
		}
		if (ImGui::MenuItem("Toggle Minimap")) {
			if (m_highlighted_room) { m_highlighted_room.value()->toggle_minimap(); }
		}
		if (ImGui::BeginMenu("Edit")) {
			if (ImGui::MenuItem("Toggle Minimap")) {
				NANI_LOG_DEBUG(p_logger, "toggled minimap!");
				if (m_highlighted_room) { m_highlighted_room.value()->toggle_minimap(); }
			}
			ImGui::EndMenu();
		}
		if (ImGui::MenuItem("Move")) {}
		if (ImGui::MenuItem("Delete")) {}
		ImGui::EndPopup();
	}
}

} // namespace pi
