
#include <imgui.h>
#include <editor/automa/DialogueEditor.hpp>
#include <editor/metagrid/tool/Cursor.hpp>
#include <editor/metagrid/tool/Move.hpp>
#include <span>
#include <imgui-SFML.h>

namespace pi {

static bool menu_open{};
static bool window_hovered{};
static bool clicked{};
static bool new_file{};
static bool open_file{};
static bool add_message{};

DialogueEditor::DialogueEditor(fornani::automa::ServiceProvider& svc) : EditorState(svc), m_tool{std::make_unique<Cursor>(svc)}, m_background_color{fornani::colors::pioneer_black}, m_data{&svc.text.npc} {
	svc.data.load_data();
	p_target_state = EditorStateType::dialogue_editor;
	p_wallpaper.setFillColor(m_background_color);
	p_wallpaper.setSize(p_services->window->f_screen_dimensions());
}

EditorStateType DialogueEditor::run(char** argv) {
	logic();

	ImGuiIO& io = ImGui::GetIO();
	window_hovered = ImGui::IsAnyItemHovered();
	io.MouseDrawCursor = menu_open || window_hovered;
	p_services->window->get().setMouseCursorVisible(io.MouseDrawCursor);

	EditorState::render(p_services->window->get());
	render(p_services->window->get());
	ImGui::SFML::Render(p_services->window->get());
	p_services->window->get().display();

	return p_target_state;
}

void DialogueEditor::handle_events(std::optional<sf::Event> event, sf::RenderWindow& win) {
	ImGuiIO& io = ImGui::GetIO();
	m_current_mouse_position = sf::Vector2f{io.MousePos.x, io.MousePos.y};
	if (auto const* key_pressed = event->getIf<sf::Event::KeyPressed>()) {
		if (key_pressed->control) {
			if (key_pressed->scancode == sf::Keyboard::Scancode::N) { new_file = true; }
			if (key_pressed->scancode == sf::Keyboard::Scancode::O) { open_file = true; }
			if (key_pressed->scancode == sf::Keyboard::Scancode::S) { save(); }
			if (key_pressed->scancode == sf::Keyboard::Scancode::M) { add_message = true; }
		}
	}
	if (auto const* button_pressed = event->getIf<sf::Event::MouseButtonPressed>()) {
		if (button_pressed->button == sf::Mouse::Button::Middle) { pressed_keys.set(PressedKeys::mouse_middle); }
		if (button_pressed->button == sf::Mouse::Button::Left) {
			if (!pressed_keys.test(PressedKeys::mouse_left)) {
				clicked = !menu_open;
				m_left_clicked_position = sf::Vector2f{m_current_mouse_position - m_camera};
			}
			pressed_keys.set(PressedKeys::mouse_left);
		}
		if (button_pressed->button == sf::Mouse::Button::Right) {
			pressed_keys.set(PressedKeys::mouse_right);
			m_right_clicked_position = m_current_mouse_position;
		}
	}
	if (auto const* button_released = event->getIf<sf::Event::MouseButtonReleased>()) {
		if (button_released->button == sf::Mouse::Button::Middle) { pressed_keys.reset(PressedKeys::mouse_middle); }
		if (button_released->button == sf::Mouse::Button::Left) {
			clicked = false;
			pressed_keys.reset(PressedKeys::mouse_left);
		}
		if (button_released->button == sf::Mouse::Button::Right) { pressed_keys.reset(PressedKeys::mouse_right); }
	}
}

void DialogueEditor::logic() {
	auto last_workspace_position = menu_open ? m_left_clicked_position : m_current_mouse_position;
	if (pressed_keys.test(PressedKeys::mouse_right)) { m_camera += m_current_mouse_position - m_dragged_position; }
	m_dragged_position = sf::Vector2f{m_current_mouse_position};
	m_tool->update(m_current_mouse_position);
}

void DialogueEditor::render(sf::RenderWindow& win) {

	// ImGui stuff

	// main toolbar
	auto PAD = 28.f;
	ImVec2 window_pos{};
	window_pos.x = p_services->window->f_screen_dimensions().x - PAD;
	window_pos.y = PAD;
	ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, {1.f, 0.f});
	if (ImGui::Begin("Info", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration)) {
		if (m_suite) {
			ImGui::Text("Host NPC: %s", m_suite->get_host().c_str());
			ImGui::Text("Tag: %s", m_suite->get_tag().c_str());
		}
		ImGui::End();
	}

	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New Dialogue Suite", "Ctrl+N")) { new_file = true; }
			if (ImGui::MenuItem("Open Dialogue Suite", "Ctrl+O")) { open_file = true; }
			ImGui::EndMenu();
		}
		if (ImGui::Button("Editor")) { p_target_state = EditorStateType::editor; }
		ImGui::EndMainMenuBar();
	}

	if (add_message) {
		static char messagebuf[512] = "";
		ImGui::OpenPopup("New Message");
		if (ImGui::BeginPopupModal("New Message", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::InputTextWithHint("Message", "message...", messagebuf, IM_ARRAYSIZE(messagebuf));
			if (ImGui::Button("Add")) {
				if (m_suite) { m_suite->add_message(messagebuf); }
				ImGui::CloseCurrentPopup();
				add_message = false;
			}
			if (ImGui::Button("Cancel")) {
				ImGui::CloseCurrentPopup();
				add_message = false;
			}
			ImGui::EndPopup();
		}
	}

	if (new_file) {
		static char hostbuf[128] = "";
		static char tagbuf[128] = "";
		ImGui::OpenPopup("New Dialogue");
		if (ImGui::BeginPopupModal("New Dialogue", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::InputTextWithHint("NPC Name", "name", hostbuf, IM_ARRAYSIZE(hostbuf));
			ImGui::InputTextWithHint("Tag", "tag", tagbuf, IM_ARRAYSIZE(tagbuf));
			if (ImGui::Button("Create")) {
				m_suite = DialogueSuite{p_services->text.fonts.basic, *m_data, hostbuf, tagbuf};
				ImGui::CloseCurrentPopup();
				new_file = false;
			}
			if (ImGui::Button("Close")) {
				ImGui::CloseCurrentPopup();
				new_file = false;
			}
			ImGui::EndPopup();
		}
	}

	if (open_file) {
		static std::string this_host{};
		static std::string this_tag{};
		ImGui::OpenPopup("Open Dialogue");
		if (ImGui::BeginPopupModal("Open Dialogue", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
			for (auto const& [host, entry] : m_data->as_object()) {
				if (ImGui::TreeNode(host.c_str())) {
					for (auto const& [conversation, entry] : entry.as_object()) {
						if (ImGui::Selectable(conversation.c_str())) {
							this_tag = conversation;
							this_host = host;
						}
					}
					ImGui::TreePop();
				}
			}
			if (ImGui::Button("Open")) {
				m_suite = DialogueSuite{p_services->text.fonts.config, *m_data, this_host, this_tag};
				ImGui::CloseCurrentPopup();
				open_file = false;
			}
			if (ImGui::Button("Close")) {
				ImGui::CloseCurrentPopup();
				open_file = false;
			}
			ImGui::EndPopup();
		}
	}

	if (m_suite) { m_suite->render(win, {-100.f, -100.f}); }
	m_tool->render(win);
}

void DialogueEditor::save() {
	if (m_suite) {
		m_suite->serialize(*m_data);
		if (!m_data->to_file((p_services->finder.paths.resources / "text" / "console" / "npc.json").string())) { NANI_LOG_ERROR(p_logger, "Failed to serialize NPC dialogue suite {}!", m_suite->get_host()); }
	}
}

} // namespace pi
