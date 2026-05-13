
#include <editor/automa/EditorContext.hpp>
#include <editor/automa/EditorState.hpp>
#include <imgui-SFML.h>

namespace pi {

EditorState::EditorState(fornani::automa::ServiceProvider& svc, EditorContext& ctx) : p_services(&svc), p_context(&ctx) { p_view = svc.window->get_view(); }

EditorStateType EditorState::run(char** argv) {
	p_left_mouse.flush();
	p_right_mouse.flush();
	p_alt.flush();
	p_control.flush();
	p_shift.flush();
	return EditorStateType::editor;
}

void EditorState::handle_events(std::optional<sf::Event> event, sf::RenderWindow& win) {
	ImGuiIO& io = ImGui::GetIO();

	sf::Vector2i mouse_pixel{static_cast<int>(io.MousePos.x), static_cast<int>(io.MousePos.y)};
	sf::Vector2f world_pos = win.mapPixelToCoords(mouse_pixel, p_view);
	p_current_mouse_position = world_pos;

	// Mouse input
	if (!io.WantCaptureMouseUnlessPopupClose) {
		if (auto const* button_pressed = event->getIf<sf::Event::MouseButtonPressed>()) {
			if (button_pressed->button == sf::Mouse::Button::Left) {
				p_left_mouse.press();
				p_left_clicked_position = p_current_mouse_position;
			}
			if (button_pressed->button == sf::Mouse::Button::Right) {
				p_right_mouse.press();
				p_right_clicked_position = p_current_mouse_position;
			}
		}
	}
	if (auto const* button_released = event->getIf<sf::Event::MouseButtonReleased>()) {
		if (button_released->button == sf::Mouse::Button::Left) {
			p_left_mouse.held = false;
			p_left_mouse.released = true;
		}
		if (button_released->button == sf::Mouse::Button::Right) {
			p_right_mouse.held = false;
			p_right_mouse.released = true;
		}
	}

	// Keyboard keys
	if (!io.WantCaptureKeyboard) {}
	if (auto const* key_pressed = event->getIf<sf::Event::KeyPressed>()) {
		if (key_pressed->scancode == sf::Keyboard::Scancode::LShift || key_pressed->scancode == sf::Keyboard::Scancode::RShift) { p_shift.press(); }
		if (key_pressed->scancode == sf::Keyboard::Scancode::LControl || key_pressed->scancode == sf::Keyboard::Scancode::RControl) { p_control.press(); }
		if (key_pressed->scancode == sf::Keyboard::Scancode::LAlt || key_pressed->scancode == sf::Keyboard::Scancode::RAlt) { p_alt.press(); }
	}
	if (auto const* key_pressed = event->getIf<sf::Event::KeyReleased>()) {
		if (key_pressed->scancode == sf::Keyboard::Scancode::LShift || key_pressed->scancode == sf::Keyboard::Scancode::RShift) { p_shift.release(); }
		if (key_pressed->scancode == sf::Keyboard::Scancode::LControl || key_pressed->scancode == sf::Keyboard::Scancode::RControl) { p_control.release(); }
		if (key_pressed->scancode == sf::Keyboard::Scancode::LAlt || key_pressed->scancode == sf::Keyboard::Scancode::RAlt) { p_alt.release(); }
	}
}

void EditorState::logic() {
	p_mouse_cooldowns.left_click.update();
	p_mouse_cooldowns.left_release.update();
	p_mouse_cooldowns.right_click.update();
	p_mouse_cooldowns.right_release.update();
	if (p_left_mouse.clicked) { p_mouse_cooldowns.left_click.start(); }
	if (p_left_mouse.released) { p_mouse_cooldowns.left_release.start(); }
	if (p_right_mouse.clicked) { p_mouse_cooldowns.right_click.start(); }
	if (p_right_mouse.released) { p_mouse_cooldowns.right_release.start(); }
	if (p_right_mouse.held) { p_camera += p_current_mouse_position - p_dragged_position; }
	p_dragged_position = p_current_mouse_position;
}

void EditorState::render(sf::RenderWindow& win) {

	// ImGui update
	ImGui::SetMouseCursor(ImGuiMouseCursor_None);
	if (!win.setActive(true)) {}
	ImGui::SFML::Update(p_services->window->get(), p_delta_clock.getElapsedTime());
	p_delta_clock.restart();

	p_services->window->get().clear();
	p_services->window->restore_view();
	p_wallpaper.setPosition({});
	p_wallpaper.setSize(sf::Vector2f{p_services->window->get_view().getSize()});
	win.draw(p_wallpaper);
}

bool EditorState::create_new_room() {
	ImGui::Text("Please enter a new room name.");
	ImGui::Text("Convention is all lowercase, snake-case, and of the format `room_name`.");
	ImGui::Separator();
	ImGui::NewLine();

	ImGui::InputTextWithHint("Region Name", "firstwind", regbuffer, IM_ARRAYSIZE(regbuffer));
	ImGui::InputTextWithHint("Room Name", "boiler_room", roombuffer, IM_ARRAYSIZE(roombuffer));
	ImGui::InputInt("ID", &p_new_id);
	ImGui::Separator();
	ImGui::NewLine();

	ImGui::Text("Please specify the dimensions of the level in chunks (16x16 tiles)");
	ImGui::Separator();
	ImGui::NewLine();

	width = ccm::ext::clamp(width, 1, std::numeric_limits<int>::max());
	height = ccm::ext::clamp(height, 1, std::numeric_limits<int>::max());

	ImGui::InputInt("Width", &width);
	ImGui::NewLine();

	ImGui::InputInt("Height", &height);
	ImGui::Separator();
	ImGui::NewLine();

	ImGui::Text("Metagrid Position");
	ImGui::InputInt("X", &p_context->metagrid_position.x);
	ImGui::SameLine();

	ImGui::InputInt("Y", &p_context->metagrid_position.y);
	ImGui::Separator();
	ImGui::NewLine();

	if (ImGui::Button("Close")) { ImGui::CloseCurrentPopup(); }
	ImGui::SameLine();
	if (ImGui::Button("Create")) { return true; }
	ImGui::EndPopup();

	return false;
}

} // namespace pi
