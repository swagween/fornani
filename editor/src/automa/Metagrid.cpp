
#include <imgui.h>
#include <editor/automa/EditorContext.hpp>
#include <editor/automa/Metagrid.hpp>
#include <editor/metagrid/tool/Cursor.hpp>
#include <editor/metagrid/tool/Move.hpp>
#include <span>
#include <imgui-SFML.h>

namespace pi {

Metagrid::Metagrid(fornani::automa::ServiceProvider& svc, EditorContext& ctx)
	: EditorState(svc, ctx), m_workspace{{256, 128}}, m_tool{std::make_unique<Cursor>(svc)}, m_background_color{fornani::colors::pioneer_black}, m_metamap{svc.assets.get_texture("metamap")} {
	svc.data.load_data();
	p_target_state = EditorStateType::metagrid;
	p_wallpaper.setFillColor(m_background_color);
	p_colors.backdrop = m_background_color;
	p_wallpaper.setSize(p_services->window->f_screen_dimensions());
	for (auto& map : svc.data.map_jsons) { m_rooms.push_back(Room(svc, map)); }

	return;

	// convert big npc file to many small ones
	for (auto const& [key, data] : svc.text.npc.as_object()) {
		auto path = std::filesystem::path{std::filesystem::path{svc.finder.resource_path()} / "localization" / "eng" / "npc"};
		auto file = key + ".json";
		auto to_json = path / file;
		if (!data.to_file(to_json.string())) {}
	}

	// here we convert legacy inspectables.
	for (auto& map : svc.data.map_jsons) {
		auto new_map = dj::Json{};
		new_map = map.metadata;
		new_map["entities"]["inspectables"] = dj::Json::empty_array();
		for (auto& inspectable : map.metadata["entities"]["inspectables"].as_array()) {
			auto new_inspectable = dj::Json{};
			new_inspectable = inspectable;

			// make new inspectables and push them to the new map

			for (auto& [key, item] : inspectable.as_object()) {
				auto old_tag = inspectable["key"].as_string() + "0";

				if (key != old_tag) { continue; }
				new_inspectable[old_tag] = dj::Json::empty_array();

				// make new json with correct structure from existing inspectable, then push it to series
				auto my_code = std::vector<std::vector<int>>{};
				for (auto& code : inspectable[old_tag]["codes"].as_array()) {
					auto arr = std::vector<int>{};
					for (auto& c : code.as_array()) { arr.push_back(c.as<int>()); }
					my_code.push_back(arr);
				}
				auto series = dj::Json{};
				auto k = 0;
				auto next = dj::Json{};
				for (auto& [tag, set] : inspectable[old_tag].as_object()) {
					k = tag == "suite" ? 0 : tag == "responses" ? 1 : 2;
					if (k > 1) { continue; }
					NANI_LOG_DEBUG(p_logger, "K was {}", k);
					for (auto [i, suite] : std::views::enumerate(inspectable[old_tag][tag].as_array())) {
						auto collection = dj::Json{};
						// we are in the set now
						for (auto [j, msg] : std::views::enumerate(suite.as_array())) {
							auto mini = dj::Json{};
							// convert text entry into small message json object
							mini["message"] = msg.as_string();
							auto clean_code = dj::Json::empty_array();
							for (auto& c : my_code) {
								if (c[0] == k && c[1] == i && c[2] == j) {
									auto arr = dj::Json::empty_array();
									for (auto [l, num] : std::views::enumerate(c)) {
										if (l > 2) { arr.push_back(num); }
									}
									clean_code.push_back(arr);
								}
							}
							mini["codes"] = clean_code;
							collection.push_back(mini);
						}
						next[tag].push_back(collection);
					}
				}
				series.push_back(next);
				new_inspectable["series"] = std::move(series);
			}
			new_map["entities"]["inspectables"].push_back(new_inspectable);
		}
		if (!new_map.to_file((svc.finder.paths.levels / std::filesystem::path{"updated"} / map.region_label / std::filesystem::path{map.room_label + ".json"}).string())) {}
	}
}

EditorStateType Metagrid::run(char** argv) {
	logic();

	ImGuiIO& io = ImGui::GetIO();
	io.MouseDrawCursor = io.WantCaptureMouse;
	p_services->window->get().setMouseCursorVisible(io.MouseDrawCursor);

	EditorState::render(p_services->window->get());
	render(p_services->window->get());
	ImGui::SFML::Render(p_services->window->get());
	p_services->window->get().display();

	EditorState::run(argv);
	return p_target_state;
}

void Metagrid::handle_events(std::optional<sf::Event> event, sf::RenderWindow& win) {
	EditorState::handle_events(event, win);
	ImGuiIO& io = ImGui::GetIO();
	if (auto const* key_pressed = event->getIf<sf::Event::KeyPressed>()) {
		if (key_pressed->scancode == sf::Keyboard::Scancode::R) {
			p_services->window->restore_view();
			p_view = p_services->window->get_view();
			p_zoom_level = 1.f;
		}
		if (key_pressed->scancode == sf::Keyboard::Scancode::LControl) { m_flags.set(MetagridFlags::control); }
	}
	if (auto const* key_pressed = event->getIf<sf::Event::KeyReleased>()) {
		if (key_pressed->scancode == sf::Keyboard::Scancode::LControl) { m_flags.reset(MetagridFlags::control); }
	}
	if (auto const* scrolled = event->getIf<sf::Event::MouseWheelScrolled>()) {
		auto zoom_rate = 0.1f;
		auto max_zoom = 3.f;
		auto min_zoom = -1.f;
		auto delta = scrolled->delta * zoom_rate;
		p_zoom_level += delta;
		auto zoom = true;
		if (p_zoom_level < min_zoom) {
			p_zoom_level = min_zoom;
			zoom = false;
		}
		if (p_zoom_level > max_zoom) {
			p_zoom_level = max_zoom;
			zoom = false;
		}
		if (zoom) { p_view.zoom(1.f - delta); }
	}
}

void Metagrid::logic() {
	EditorState::logic();
	ImGuiIO& io = ImGui::GetIO();

	auto cursorpos = sf::Vector2f{io.MousePos.x, io.MousePos.y};
	sf::Vector2f relative_mouse = p_current_mouse_position - p_camera;

	auto last_workspace_position = io.WantCaptureMouse ? p_left_clicked_position : p_current_mouse_position;

	// mouse events
	if (p_left_mouse.released && m_highlighted_room && m_tool->is(MetagridToolType::move)) {
		if (!m_highlighted_room.value()->serialize(*p_services)) { NANI_LOG_INFO(p_logger, "Failed to save metadata for {}", m_highlighted_room.value()->get_label()); }
	}
	if (p_left_mouse.held && m_highlighted_room) {
		m_flags.set(MetagridFlags::move_mode);
		if (m_tool->is(MetagridToolType::move)) { m_highlighted_room.value()->set_position(m_tool->get_workspace_coordinates(relative_mouse)); }
		p_left_clicked_position = p_current_mouse_position - p_camera;
		m_tool->set_original_position(m_highlighted_room.value()->get_board_position());
	} else {
		m_flags.reset(MetagridFlags::move_mode);
	}

	auto any_room_hovered = false;
	for (auto& r : m_rooms) {
		r.update(last_workspace_position, io.WantCaptureMouse);
		if (r.is_highlighted()) {
			any_room_hovered = true;
			if (m_flags.test(MetagridFlags::control)) {
				if (p_left_mouse.clicked) { r.increment_status(*p_services); }
			}
		}
	}
	m_tool->update(p_current_mouse_position);
	if (p_left_mouse.clicked) {
		if (!any_room_hovered && !io.WantCaptureMouse) {
			m_flags.set(MetagridFlags::context_menu);
			p_context->metagrid_position = m_tool->get_workspace_coordinates(relative_mouse);
		}
	}
	any_room_hovered || io.WantCaptureMouse ? m_flags.set(MetagridFlags::hide_cell) : m_flags.reset(MetagridFlags::hide_cell);
}

void Metagrid::render(sf::RenderWindow& win) {
	ImGuiIO& io = ImGui::GetIO();
	auto cursorpos = sf::Vector2f{io.MousePos.x, io.MousePos.y};
	// calculate zoom
	sf::View const& cview = win.getView();
	float zoom = cview.getSize().x / win.getDefaultView().getSize().x;
	float base_thickness = -2.f;

	p_services->window->set_view(p_view);
	if (p_zoom_level >= 1.f) { m_workspace.render(win, p_camera); }

	m_metamap_settings.color.a = static_cast<unsigned char>(m_metamap_settings.alpha);
	m_metamap.setColor(m_metamap_settings.color);
	m_metamap.setPosition(m_metamap_settings.position + p_camera);
	m_metamap.setScale({m_metamap_settings.scale, m_metamap_settings.scale});
	if (m_metamap_settings.show) { win.draw(m_metamap); }

	sf::Vector2f relative_mouse = p_current_mouse_position - p_camera;
	sf::Vector2f grid_pos = relative_mouse / spacing_v;

	m_current_cell.setPosition(sf::Vector2f{m_tool->get_workspace_coordinates(relative_mouse)} * spacing_v + p_camera);
	m_current_cell.setSize({spacing_v, spacing_v});
	m_current_cell.setOutlineColor(fornani::colors::dark_grey);
	m_current_cell.setFillColor(fornani::colors::transparent);
	m_current_cell.setOutlineThickness(base_thickness * zoom);
	if (!m_flags.test(MetagridFlags::hide_cell)) { win.draw(m_current_cell); }

	static auto current_room = 0;
	static bool serialize{};
	static bool ignore_test_levels{true};
	static bool hide_room_borders{};
	static bool show_tags{};

	// render rooms
	auto found_one{false};
	auto it = 0;
	auto ctr = 0;
	for (auto& r : m_rooms) {
		r.no_border = hide_room_borders;
		r.show_tags = show_tags;
		if (!r.has_flag_set(RoomFlags::include_in_minimap) && ignore_test_levels) {
			++ctr;
			continue;
		}
		if (r.is_highlighted()) {
			found_one = true;
			current_room = r.id.get();
			it = ctr;
		}
		r.render(win, p_camera);
		++ctr;
	}
	auto view = std::span<Room>(m_rooms);
	if (!p_left_mouse.clicked && !io.WantCaptureMouse && !m_flags.test(MetagridFlags::move_mode)) { m_highlighted_room = &view[it]; }
	if (!found_one && !m_flags.test(MetagridFlags::move_mode) && !io.WantCaptureMouse) { m_highlighted_room.reset(); }

	if (io.WantCaptureMouse) {
		p_services->window->restore_view();
		auto screen = p_wallpaper;
		screen.setFillColor(sf::Color{100, 100, 100, 20});
		win.draw(screen);
	}

	// ImGui stuff
	bool options_popup{p_left_mouse.clicked && found_one && m_tool->is(MetagridToolType::cursor) && !m_flags.test(MetagridFlags::control)};
	if (ImGui::BeginMainMenuBar()) {
		if (ImGui::BeginMenu("File")) { ImGui::EndMenu(); }
		if (ImGui::Button("Editor")) { p_target_state = EditorStateType::editor; }
		ImGui::EndMainMenuBar();
	}

	if (m_flags.test(MetagridFlags::context_menu)) {
		ImGui::OpenPopup("Context Menu");
		m_flags.reset(MetagridFlags::context_menu);
	}
	if (ImGui::BeginPopupContextWindow("Context Menu")) {
		if (ImGui::MenuItem("New Room")) {
			p_target_state = EditorStateType::editor;
			p_context->flags.set(EditorContextFlags::new_room);
		}
		ImGui::EndPopup();
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
		ImGui::Text("Zoom Level: %.3f", p_zoom_level);
		ImGui::Text("Workspace Coordinates: (%i, %i)", m_tool->get_workspace_coordinates(relative_mouse).x, m_tool->get_workspace_coordinates(relative_mouse).y);
		if (m_flags.test(MetagridFlags::move_mode)) { ImGui::Text("MOVE MODE"); }
		ImGui::Checkbox("Ignore Test Levels", &ignore_test_levels);
		ImGui::Checkbox("Hide Room Borders", &hide_room_borders);
		ImGui::Checkbox("Show Tags", &show_tags);

		ImGui::SeparatorText("Metamap");
		ImGui::Checkbox("Show", &m_metamap_settings.show);
		ImGui::SliderFloat("X Position", &m_metamap_settings.position.x, -1204.f, -1184, "%.1f");
		ImGui::SliderFloat("Y Position", &m_metamap_settings.position.y, -1450, -1430, "%.1f");
		// ImGui::SliderFloat("Scale", &m_metamap_settings.scale, 0.f, 2.f, "%.1f");
		ImGui::SliderInt("Opacity", &m_metamap_settings.alpha, 0, 255);

		ImGui::End();
	}

	if (m_highlighted_room && found_one) {
		ImGui::BeginTooltip();
		ImGui::Text("%s", m_highlighted_room.value()->get_label().c_str());
		ImGui::Separator();
		ImGui::Text("ID: %i", m_highlighted_room.value()->id.get());
		ImGui::Text("Biome: %s", m_highlighted_room.value()->get_biome().c_str());
		ImGui::Text("Music: %s", m_highlighted_room.value()->get_music().c_str());
		ImGui::Text("Interior: %s", m_highlighted_room.value()->has_flag_set(RoomFlags::interior) ? "Yes" : "No");
		if (m_highlighted_room.value()->get_data().metadata["meta"]["weather"]) {
			ImGui::SeparatorText("Weather");
			ImGui::Text("Type: %s", m_highlighted_room.value()->get_data().metadata["meta"]["weather"]["type"].as_string().c_str());
			ImGui::Text("Chance: %.1f", m_highlighted_room.value()->get_data().metadata["meta"]["weather"]["chance"].as<float>());
		}
		ImGui::EndTooltip();
	}

	if (options_popup) { ImGui::OpenPopup("Room Options"); }
	if (ImGui::BeginPopupContextWindow("Room Options")) {
		auto ut = m_highlighted_room ? m_highlighted_room.value()->has_flag_set(RoomFlags::use_template) : false;
		if (ImGui::BeginMenu("Edit")) {
			if (ImGui::MenuItem("Toggle Minimap")) {
				if (m_highlighted_room) {
					m_highlighted_room.value()->toggle_flag(RoomFlags::include_in_minimap);
					serialize = true;
				}
			}
			if (ImGui::MenuItem("Toggle Interior")) {
				if (m_highlighted_room) {
					m_highlighted_room.value()->toggle_flag(RoomFlags::interior);
					serialize = true;
				}
			}
			if (ImGui::MenuItem("Toggle DayNightShift")) {
				if (m_highlighted_room) {
					m_highlighted_room.value()->toggle_flag(RoomFlags::day_night_shift);
					serialize = true;
				}
			}
			if (ImGui::Checkbox("Use Template", &ut)) {
				if (m_highlighted_room) {
					m_highlighted_room.value()->toggle_flag(RoomFlags::use_template);
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
	}

	if (serialize && m_highlighted_room) {
		if (!m_highlighted_room.value()->serialize(*p_services)) { NANI_LOG_INFO(p_logger, "Failed to save metadata for {}", m_highlighted_room.value()->get_label()); }
		serialize = false;
	}

	p_services->window->restore_view();
	m_tool->render(win, sf::Vector2f{io.MousePos});
	p_services->window->set_view(p_view);
}

} // namespace pi
