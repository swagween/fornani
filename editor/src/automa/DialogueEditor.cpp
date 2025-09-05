
#include <imgui.h>
#include <editor/automa/DialogueEditor.hpp>
#include <editor/metagrid/tool/Cursor.hpp>
#include <editor/metagrid/tool/Move.hpp>
#include <span>
#include <imgui-SFML.h>

namespace pi {

static bool window_hovered{};
static bool clicked{};
static bool new_file{};
static bool open_file{};
static bool add_message{};
static bool edit_message{};
static bool add_code{};
static bool is_any_node_hovered{};
static bool is_any_node_selected{};

DialogueEditor::DialogueEditor(fornani::automa::ServiceProvider& svc) : EditorState(svc), m_tool{std::make_unique<Cursor>(svc)}, m_background_color{fornani::colors::pioneer_black}, m_data{&svc.text.npc} {
	svc.data.load_data();
	p_target_state = EditorStateType::dialogue_editor;
	p_wallpaper.setFillColor(m_background_color);
	p_wallpaper.setSize(p_services->window->f_screen_dimensions());
}

EditorStateType DialogueEditor::run(char** argv) {
	logic();

	ImGuiIO& io = ImGui::GetIO();
	window_hovered = io.WantCaptureMouse;
	io.MouseDrawCursor = window_hovered;
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
				if (m_suite) { m_suite->deselect_all(); }
				clicked = !window_hovered;
				m_left_clicked_position = sf::Vector2f{m_current_mouse_position - m_camera};
				if (m_suite) { is_any_node_hovered = m_suite->is_any_node_hovered(); }
				if (m_suite) { is_any_node_selected = m_suite->is_any_node_selected(); }
				if (!is_any_node_hovered) { clicked = false; }
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
	if (auto const* scrolled = event->getIf<sf::Event::MouseWheelScrolled>()) {
		auto zoom_factor = 10.f;
		auto delta = scrolled->delta * zoom_factor;
		m_camera.y -= delta;
	}
}

void DialogueEditor::logic() {
	auto last_workspace_position = window_hovered ? m_left_clicked_position : m_current_mouse_position;
	if (pressed_keys.test(PressedKeys::mouse_right)) { m_camera -= m_current_mouse_position - m_dragged_position; }
	m_dragged_position = sf::Vector2f{m_current_mouse_position};
	m_tool->update(m_current_mouse_position);
	if (m_suite) { m_suite->update(m_current_mouse_position, clicked); }
}

void DialogueEditor::render(sf::RenderWindow& win) {
	// ImGui stuff
	bool options_popup{clicked && m_tool->is(MetagridToolType::cursor) && is_any_node_hovered};
	// main toolbar
	auto PAD = 28.f;
	ImVec2 window_pos{};
	window_pos.x = p_services->window->f_screen_dimensions().x - PAD;
	window_pos.y = PAD;
	ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, {1.f, 0.f});
	if (ImGui::Begin("Info", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration)) {
		if (m_suite) {
			ImGui::SeparatorText("General");
			ImGui::Text("Host NPC: %s", m_suite->get_host().c_str());
			ImGui::Text("Tag: %s", m_suite->get_tag().c_str());
			ImGui::Text("Current Set: %i", m_suite->get_current_set());
			ImGui::Text("Current Index: %i", m_suite->get_current_index());
			ImGui::SeparatorText("Codes");
			m_suite->print_codes();
			ImGui::Separator();
			if (ImGui::TreeNode("Debug Flags")) {
				ImGui::Text("Any Node Hovered: %s", is_any_node_hovered ? "True" : "False");
				ImGui::Text("Any Node Selected: %s", is_any_node_selected ? "True" : "False");
				ImGui::Text("Window Hovered: %s", window_hovered ? "True" : "False");
				ImGui::TreePop();
			}
		}
		static std::string this_host{};
		static std::string this_tag{};
		ImGui::SeparatorText("Library");
		if (ImGui::TreeNode("Open Dialogue")) {
			for (auto const& [host, entry] : m_data->as_object()) {
				if (ImGui::TreeNode(host.c_str())) {
					for (auto const& [conversation, entry] : entry.as_object()) {
						if (ImGui::Selectable(conversation.c_str())) {
							this_tag = conversation;
							this_host = host;
							m_suite = DialogueSuite{p_services->text.fonts.basic, *m_data, this_host, this_tag};
							ImGui::CloseCurrentPopup();
							open_file = false;
						}
					}
					ImGui::TreePop();
				}
			}
			ImGui::TreePop();
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

	if (ImGui::BeginPopupContextWindow("Node Options")) {
		if (ImGui::MenuItem("Edit")) {
			if (m_suite) {
				if (m_current_node = m_suite->get_current_node()) { edit_message = true; }
			}
		}
		if (ImGui::MenuItem("Add Code")) {
			if (m_suite) {
				if (m_current_node = m_suite->get_current_node()) { add_code = true; }
			}
		}
		ImGui::EndPopup();
	}

	if (add_message) {
		static char messagebuf[512] = "";
		ImGui::OpenPopup("New Message");
		if (ImGui::BeginPopupModal("New Message", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::InputTextWithHint("Message", "message...", messagebuf, IM_ARRAYSIZE(messagebuf));
			if (ImGui::Button("Add")) {
				if (m_suite) { m_suite->add_message(messagebuf, NodeType::suite); }
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

	if (add_code) {
		ImGui::OpenPopup("Add Code");
		add_code = false;
	}
	static char messagebuf[512] = "";
	if (edit_message && is_any_node_selected) {
		ImGui::OpenPopup("Edit Message");
		std::strncpy(messagebuf, m_current_node->get_message().c_str(), sizeof(messagebuf) - 1);
		messagebuf[sizeof(messagebuf) - 1] = '\0';
		edit_message = false;
	}
	if (m_current_node) {
		if (ImGui::BeginPopupModal("Edit Message", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
			options_popup = false;
			ImGui::InputTextWithHint("Message", "message...", messagebuf, IM_ARRAYSIZE(messagebuf));
			if (ImGui::Button("Apply")) {
				auto to_node = Node(p_services->text.fonts.basic, messagebuf, m_current_node->get_type());
				if (m_suite) { m_suite->swap_node(to_node); }
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::Button("Cancel")) { ImGui::CloseCurrentPopup(); }
			ImGui::EndPopup();
		}
	} // tool node selection options
	if (ImGui::BeginPopupContextWindow("Add Code")) {
		static auto ri = 0;
		options_popup = false;
		if (ImGui::TreeNode("Response")) {
			if (m_suite) {
				ImGui::InputInt("Response Index", &ri);
				if (ImGui::Button("Add")) {
					m_suite->add_code(fornani::gui::MessageCodeType::response, ri);
					ImGui::CloseCurrentPopup();
				}
			}
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Special")) {
			if (m_suite) {
				ImGui::SeparatorText("NPC Emotion");
				ImGui::InputInt("Emotion", &ri);
				if (ImGui::Button("Add##a")) {
					m_suite->add_code(fornani::gui::MessageCodeType::emotion, ri);
					ImGui::CloseCurrentPopup();
				}
				ImGui::SeparatorText("Pop Conversation");
				ImGui::InputInt("NPC ID", &ri);
				if (ImGui::Button("Add##b")) {
					m_suite->add_code(fornani::gui::MessageCodeType::pop_conversation, ri);
					ImGui::CloseCurrentPopup();
				}
				ImGui::SeparatorText("Give Item");
				for (auto const& item : p_services->data.item.as_object()) {
					if (ImGui::Selectable(item.first.c_str())) {
						ri = item.second["id"].as<int>();
						ImGui::SetItemDefaultFocus();
					}
				}
				if (ImGui::Button("Add##c")) {
					m_suite->add_code(fornani::gui::MessageCodeType::item, ri);
					ImGui::CloseCurrentPopup();
				}
			}
			ImGui::TreePop();
		}
		if (ImGui::Button("Cancel")) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}
	if (options_popup) {
		if (m_suite) {
			ImGui::OpenPopup("Node Options");
			clicked = false;
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
							m_suite = DialogueSuite{p_services->text.fonts.basic, *m_data, this_host, this_tag};
							ImGui::CloseCurrentPopup();
							open_file = false;
						}
					}
					ImGui::TreePop();
				}
			}
			if (ImGui::Button("Close")) {
				ImGui::CloseCurrentPopup();
				open_file = false;
			}
			ImGui::EndPopup();
		}
	}

	if (m_suite) {
		if (window_hovered) { m_suite->unhover_all(); }
		m_suite->render(win, m_camera);
	}
	m_tool->render(win);
}

void DialogueEditor::save() {
	if (m_suite) {
		m_suite->serialize(*m_data);
		if (!m_data->to_file((p_services->finder.paths.resources / "text" / "console" / "new_npc.json").string())) { NANI_LOG_ERROR(p_logger, "Failed to serialize NPC dialogue suite {}!", m_suite->get_host()); }
	}
}

} // namespace pi
