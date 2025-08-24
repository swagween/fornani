
#include <imgui.h>
#include <editor/automa/Metagrid.hpp>
#include <editor/metagrid/tool/Cursor.hpp>
#include <editor/metagrid/tool/Move.hpp>
#include <span>
#include <imgui-SFML.h>

namespace pi {

static bool menu_open{};
static bool window_hovered{};
static bool clicked{};

Metagrid::Metagrid(fornani::automa::ServiceProvider& svc) : EditorState(svc), m_workspace{{256, 128}}, m_tool{std::make_unique<Cursor>(svc)} {
	// svc.data.load_data();
	p_target_state = EditorStateType::metagrid;
	p_wallpaper.setFillColor(m_background_color);
	p_wallpaper.setSize(p_services->window->f_screen_dimensions());
	for (auto& map : svc.data.map_jsons) { m_rooms.push_back(Room(svc, map)); }
}

EditorStateType Metagrid::run(char** argv) {
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

void Metagrid::handle_events(std::optional<sf::Event> event, sf::RenderWindow& win) {
	ImGuiIO& io = ImGui::GetIO();
	m_current_mouse_position = sf::Vector2f{io.MousePos.x, io.MousePos.y};
	if (auto const* button_pressed = event->getIf<sf::Event::MouseButtonPressed>()) {
		if (button_pressed->button == sf::Mouse::Button::Middle) { pressed_keys.set(PressedKeys::mouse_middle); }
		if (button_pressed->button == sf::Mouse::Button::Left) {
			if (!pressed_keys.test(PressedKeys::mouse_left) && m_highlighted_room) {
				clicked = !menu_open;
				m_left_clicked_position = sf::Vector2f{m_current_mouse_position - m_camera};
				m_tool->set_original_position(m_highlighted_room.value()->get_board_position());
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
			if (pressed_keys.test(PressedKeys::mouse_left) && m_highlighted_room && m_tool->is(MetagridToolType::move)) {
				if (!m_highlighted_room.value()->serialize(*p_services)) { NANI_LOG_INFO(p_logger, "Failed to save metadata for {}", m_highlighted_room.value()->get_label()); }
			}
			clicked = false;
			pressed_keys.reset(PressedKeys::mouse_left);
		}
		if (button_released->button == sf::Mouse::Button::Right) { pressed_keys.reset(PressedKeys::mouse_right); }
	}
	if (m_highlighted_room && pressed_keys.test(PressedKeys::mouse_left)) { m_tool->handle_inputs(*m_highlighted_room.value(), m_camera, m_left_clicked_position); }
}

void Metagrid::logic() {
	auto last_workspace_position = menu_open ? m_left_clicked_position : m_current_mouse_position;
	if (pressed_keys.test(PressedKeys::mouse_right)) { m_camera += m_current_mouse_position - m_dragged_position; }
	m_dragged_position = sf::Vector2f{m_current_mouse_position};
	for (auto& r : m_rooms) { r.update(last_workspace_position); }
	m_tool->update(m_current_mouse_position);
}

void Metagrid::render(sf::RenderWindow& win) {
	m_workspace.render(win, m_camera);

	static auto current_room = 0;
	static bool serialize{};
	static bool ignore_test_levels{true};

	// render rooms
	auto found_one{false};
	auto it = 0;
	auto ctr = 0;
	for (auto& r : m_rooms) {
		if (!r.is_minimap() && ignore_test_levels) {
			++ctr;
			continue;
		}
		if (r.is_highlighted()) {
			found_one = true;
			current_room = r.id.get();
			it = ctr;
		}
		r.render(win, m_camera);
		++ctr;
	}
	auto view = std::span<Room>(m_rooms);
	if (!pressed_keys.test(PressedKeys::mouse_left) && !menu_open && !window_hovered) { m_highlighted_room = &view[it]; }

	m_tool->render(win);

	// ImGui stuff
	bool options_popup{clicked && found_one && m_tool->is(MetagridToolType::cursor)};
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) { ImGui::EndMenu(); }
		if (ImGui::Button("Editor")) { p_target_state = EditorStateType::editor; }
		ImGui::EndMainMenuBar();
	}

	// main toolbar
	auto PAD = 28.f;
	ImVec2 window_pos{};
	window_pos.x = p_services->window->f_screen_dimensions().x - PAD;
	window_pos.y = PAD;
	ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, {1.f, 0.f});
	if (ImGui::Begin("Info", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration)) {

		auto window_flags = ImGuiWindowFlags_None | ImGuiWindowFlags_MenuBar;
		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);

		ImGui::BeginChild("ChildR", ImVec2(320, 72), true, window_flags);
		ImGui::BeginMenuBar();
		if (ImGui::BeginMenu("Tools")) { ImGui::EndMenu(); }

		ImGui::EndMenuBar();
		auto tools = sf::Sprite{p_services->assets.get_texture("editor_tools")};
		tools.setScale(sf::Vector2f{constants::ui_tool_scale_v, constants::ui_tool_scale_v});
		for (int i = 0; i < static_cast<int>(MetagridToolType::END); ++i) {
			ImGui::PushID(i);
			tools.setTextureRect(sf::IntRect{{(i + static_cast<int>(ToolType::END)) * constants::tool_size_v, 0}, {constants::tool_size_v, constants::tool_size_v}});
			ImGui::ImageButton(std::to_string(i).c_str(), tools, ImVec2{constants::tool_size_v * constants::ui_tool_scale_v, constants::tool_size_v * constants::ui_tool_scale_v}, sf::Color::Transparent, sf::Color::White);
			if (ImGui::IsItemClicked()) {
				switch (static_cast<MetagridToolType>(i)) {
				case MetagridToolType::cursor: m_tool = std::move(std::make_unique<Cursor>(*p_services)); break;
				case MetagridToolType::move: m_tool = std::move(std::make_unique<Move>(*p_services)); break;
				default: m_tool = std::move(std::make_unique<Cursor>(*p_services)); break;
				}
			}
			ImGui::PopID();
			ImGui::SameLine();
		}

		ImGui::EndChild();
		ImGui::PopStyleVar();

		ImGui::Separator();
		ImGui::Text("Current Room: %i", current_room);
		ImGui::Text("Current Tool: %s", m_tool->get_label().data());
		ImGui::Text("Workspace Coordinates: (%i, %i)", m_tool->get_workspace_coordinates(m_camera).x, m_tool->get_workspace_coordinates(m_camera).y);
		ImGui::Checkbox("Ignore Test Levels", &ignore_test_levels);

		ImGui::End();
	}

	if (m_highlighted_room && found_one) {
		ImGui::BeginTooltip();
		ImGui::Text("%s", m_highlighted_room.value()->get_label().c_str());
		ImGui::Separator();
		ImGui::Text("ID: %i", m_highlighted_room.value()->id.get());
		ImGui::Text("Biome: %s", m_highlighted_room.value()->get_biome().c_str());
		ImGui::EndTooltip();
	}

	if (options_popup) {
		ImGui::OpenPopup("Room Options");
		clicked = false;
	}
	if (ImGui::BeginPopupContextWindow("Room Options")) {
		menu_open = true;
		if (ImGui::BeginMenu("Edit")) {
			if (ImGui::MenuItem("Toggle Minimap")) {
				if (m_highlighted_room) {
					m_highlighted_room.value()->toggle_minimap();
					serialize = true;
				}
			}
			ImGui::EndMenu();
		}
		if (ImGui::MenuItem("Open")) {
			if (m_highlighted_room) {
				p_target_state = EditorStateType::editor;
				p_services->finder.paths.region = m_highlighted_room.value()->get_region();
				p_services->finder.paths.room_name = m_highlighted_room.value()->get_filename();
			}
		}
		ImGui::EndPopup();
	} else {
		menu_open = false;
	}

	if (serialize && m_highlighted_room) {
		if (!m_highlighted_room.value()->serialize(*p_services)) { NANI_LOG_INFO(p_logger, "Failed to save metadata for {}", m_highlighted_room.value()->get_label()); }
		serialize = false;
	}
}

} // namespace pi
