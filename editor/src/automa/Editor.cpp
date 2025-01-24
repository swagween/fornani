#include <algorithm>
#include "fornani/setup/ResourceFinder.hpp"
#include "fornani/setup/Application.hpp"
#include "editor/util/Lookup.hpp"
#include "editor/automa/Editor.hpp"
#include "editor/gui/Console.hpp"
#include <filesystem>

namespace pi {

Editor::Editor(char** argv, WindowManager& window, data::ResourceFinder& finder) : window(&window), finder(&finder), current_tool(std::make_unique<Hand>()), secondary_tool(std::make_unique<Hand>()) {
	args = argv;
	std::cout << "Level path: " << finder.paths.levels << "\n";

	if (!tool_texture.loadFromFile((finder.paths.editor / "gui" / "tools.png").string())) { std::cout << "Failed to load tool texture.\n"; }

	for (int i = 0; i < static_cast<int>(Style::END); ++i) {
		char const* next = get_style_string.at(static_cast<Style>(i));
		styles[i] = next;
	}
	for (int i = 0; i < static_cast<int>(Backdrop::END); ++i) {
		char const* next = get_backdrop_string.at(static_cast<Backdrop>(i));
		bgs[i] = next;
	}

	finder.paths.room_name = "new_file";
	std::string msg = "Loading room: " + finder.paths.room_name;
	console.add_log(msg.data());
	load();
	map.get_layers().layers.at(MIDDLEGROUND).active = true;
	palette.flags.show_entities = false;

	target_shape.setFillColor(sf::Color{110, 90, 200, 80});
	target_shape.setOutlineColor(sf::Color{240, 230, 255, 100});
	target_shape.setOutlineThickness(-2);
	target_shape.setSize({map.f_cell_size(), map.f_cell_size()});

	center_map();
}

void Editor::run() {

	console.add_log("Welcome to Pioneer!");

	// load the tilesets!
	for (int i = 0; i < static_cast<int>(Style::END); ++i) {
		tileset_textures.push_back(sf::Texture());
		std::string style = get_style_string.at(static_cast<Style>(i));
		std::string filename = style + "_tiles.png";
		if (!tileset_textures.back().loadFromFile((finder->paths.resources / "image" / "tile" / filename).string())) { console.add_log(std::string{"Failed to load " + filename}.c_str()); }
	}

	init("new_file");
	
	bool debug_mode = false;

	wallpaper.setSize(window->f_screen_dimensions());
	colors.backdrop = sf::Color{40, 60, 80};
	wallpaper.setFillColor(colors.backdrop);
	sf::Clock delta_clock{};

	// editor loop
	while (window->get().isOpen()) {

		if (flags.test(GlobalFlags::shutdown)) { return; }

		if (trigger_demo) {
			auto ppos = static_cast<sf::Vector2<float>>(map.entities.variables.player_start) * 32.f;
			launch_demo(args, map.room_id, finder->paths.room_name, ppos);
			ImGui::SFML::Init(window->get());
			init(finder->paths.room_name);
		}

		// events
		while (std::optional const event = window->get().pollEvent()) {
			ImGui::SFML::ProcessEvent(window->get(), *event);
			if (event->is<sf::Event::Closed>()) { return; }
			handle_events(*event, window->get());
		}

		// game logic and rendering
		logic();

		// ImGui update
		ImGui::SetMouseCursor(ImGuiMouseCursor_None);
		ImGuiIO& io = ImGui::GetIO();
		io.MouseDrawCursor = window_hovered || menu_hovered;
		window->get().setMouseCursorVisible(io.MouseDrawCursor);
		ImGui::SFML::Update(window->get(), delta_clock.getElapsedTime());
		delta_clock.restart();

		window->get().clear();
		wallpaper.setFillColor(colors.backdrop);
		window->get().draw(wallpaper);

		render(window->get());

		ImGui::SFML::Render(window->get());
		window->get().display();
	}
}

void Editor::init(std::string const& load_path) {}

void Editor::handle_events(std::optional<sf::Event> const event, sf::RenderWindow& win) {

	auto& source = palette_mode() || current_tool->has_palette_selection ? palette : map;

	// keyboard events
	if (auto const* key_pressed = event->getIf<sf::Event::KeyPressed>()) {
		if (!menu_hovered && !popup_open) {
			if (key_pressed->scancode == sf::Keyboard::Scancode::A) { current_tool->change_size(-1); }
			if (key_pressed->scancode == sf::Keyboard::Scancode::D) { current_tool->change_size(1); }
			if (key_pressed->scancode == sf::Keyboard::Scancode::R) { center_map(); }
			if (key_pressed->scancode == sf::Keyboard::Scancode::H) { current_tool = std::move(std::make_unique<Hand>()); }
			if (key_pressed->scancode == sf::Keyboard::Scancode::B) { current_tool = std::move(std::make_unique<Brush>()); }
			if (key_pressed->scancode == sf::Keyboard::Scancode::G) { current_tool = std::move(std::make_unique<Fill>()); }
			if (key_pressed->scancode == sf::Keyboard::Scancode::E) { current_tool = std::move(std::make_unique<Erase>()); }
			if (key_pressed->scancode == sf::Keyboard::Scancode::M) { current_tool = std::move(std::make_unique<Marquee>()); }
			if (key_pressed->scancode == sf::Keyboard::Scancode::N) { current_tool = std::move(std::make_unique<EntityEditor>()); }
		}
		if (control_pressed()) {
			if (key_pressed->scancode == sf::Keyboard::Scancode::X) { current_tool->handle_keyboard_events(source, key_pressed->scancode); }
			if (key_pressed->scancode == sf::Keyboard::Scancode::C) { current_tool->handle_keyboard_events(source, key_pressed->scancode); }
			if (key_pressed->scancode == sf::Keyboard::Scancode::V && !palette_mode()) { current_tool->handle_keyboard_events(map, key_pressed->scancode); }
			if (key_pressed->scancode == sf::Keyboard::Scancode::R) {
				map.center(window->f_center_screen());
				map.set_scale(1.f);
			}
			if (key_pressed->scancode == sf::Keyboard::Scancode::D) { trigger_demo = true; }
		}
		if (key_pressed->scancode == sf::Keyboard::Scancode::Q) { current_tool->handle_keyboard_events(map, key_pressed->scancode); }
		if (key_pressed->scancode == sf::Keyboard::Scancode::LShift || key_pressed->scancode == sf::Keyboard::Scancode::RShift) { pressed_keys.set(PressedKeys::shift); }
		if (key_pressed->scancode == sf::Keyboard::Scancode::LControl || key_pressed->scancode == sf::Keyboard::Scancode::RControl) { pressed_keys.set(PressedKeys::control); }
		if (key_pressed->scancode == sf::Keyboard::Scancode::Space) { pressed_keys.set(PressedKeys::space); }
		if (key_pressed->scancode == sf::Keyboard::Scancode::LAlt) {
			if (current_tool->type == ToolType::brush) { current_tool = std::move(std::make_unique<Eyedropper>()); }
		}
		if (key_pressed->scancode == sf::Keyboard::Scancode::Z) {
			if (control_pressed() && !shift_pressed()) { map.undo(); }
			if (control_pressed() && shift_pressed()) { map.redo(); }
		}
	}

	if (auto const* key_released = event->getIf<sf::Event::KeyReleased>()) {
		if (key_released->scancode == sf::Keyboard::Scancode::LShift || key_released->scancode == sf::Keyboard::Scancode::RShift) { pressed_keys.reset(PressedKeys::shift); }
		if (key_released->scancode == sf::Keyboard::Scancode::LControl || key_released->scancode == sf::Keyboard::Scancode::RControl) { pressed_keys.reset(PressedKeys::control); }
		if (key_released->scancode == sf::Keyboard::Scancode::Space) { pressed_keys.reset(PressedKeys::space); }
		if (key_released->scancode == sf::Keyboard::Scancode::LAlt) {
			if (current_tool->type == ToolType::eyedropper) { current_tool = std::move(std::make_unique<Brush>()); }
		}
	}

	// zoom controls
	if (available()) {
		if (auto const* scrolled = event->getIf<sf::Event::MouseWheelScrolled>()) {
			auto delta = scrolled->delta * zoom_factor * map.get_scale();
			if (map.within_zoom_limits(delta)) { map.move(current_tool->f_position() * -delta); }
			map.zoom(delta);
		}
	}

	// mouse events
	if (auto const* button_pressed = event->getIf<sf::Event::MouseButtonPressed>()) {
		if (button_pressed->button == sf::Mouse::Button::Left) { pressed_keys.set(PressedKeys::mouse_left); }
		if (button_pressed->button == sf::Mouse::Button::Right) { pressed_keys.set(PressedKeys::mouse_right); }
		if (left_mouse_pressed()) { current_tool->click(); }
		if (right_mouse_pressed()) { secondary_tool->click(); }
	}
	if (auto const* button_released = event->getIf<sf::Event::MouseButtonReleased>()) {
		if (left_mouse_pressed()) { current_tool->unsuppress(); }
		if (right_mouse_pressed()) { secondary_tool->unsuppress(); }
		if (left_mouse_pressed()) { current_tool->neutralize(); }
		if (right_mouse_pressed()) { secondary_tool->neutralize(); }
		if (left_mouse_pressed()) { current_tool->release(); }
		if (right_mouse_pressed()) { secondary_tool->release(); }
		if (button_released->button == sf::Mouse::Button::Left) { pressed_keys.reset(PressedKeys::mouse_left); }
		if (button_released->button == sf::Mouse::Button::Right) { pressed_keys.reset(PressedKeys::mouse_right); }
	}
}

void Editor::logic() {
	window_hovered = ImGui::IsAnyItemHovered() || ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) || ImGui::IsAnyItemActive();
	current_tool->palette_mode = palette_mode();

	// tool logic
	auto& tool = left_mouse_pressed() ? current_tool : secondary_tool;
	auto& target = palette_mode() ? palette : map;
	if (available()) { map.save_state(*tool); }

	left_mouse_pressed() && current_tool->is_ready() && available() ? current_tool->activate() : current_tool->deactivate();
	right_mouse_pressed() && secondary_tool->is_ready() && available() ? secondary_tool->activate() : secondary_tool->deactivate();
	tool->update(target);

	if ((any_mouse_pressed()) && !menu_hovered && !window_hovered) {
		if (tool->type == ToolType::eyedropper) { selected_block = current_tool->tile; }
		if (palette_mode() && current_tool->type != ToolType::marquee) {
			auto pos = current_tool->get_window_position() - palette.get_position();
			auto idx = palette.tile_val_at_scaled(static_cast<int>(pos.x), static_cast<int>(pos.y), 4);
			current_tool->store_tile(idx);
			selected_block = idx;
			if (!current_tool->is_paintable()) { current_tool = std::move(std::make_unique<Brush>()); }
		}
	}

	palette.active_layer = 4;
	map.active_layer = active_layer;
	if (current_tool->trigger_switch) { current_tool = std::move(std::make_unique<Hand>()); }
	current_tool->tile = selected_block;
	current_tool->pervasive = tool_flags.pervasive;
	current_tool->contiguous = tool_flags.contiguous;
	current_tool->set_usability(current_tool->in_bounds(map.dimensions));

	map.update(*current_tool);
	palette.update(*current_tool);
	palette.set_position({12.f, 32.f});
	if (palette.hovered()) { map.unhover(); }

	map.set_offset_from_center(map.get_position() + map.get_scaled_center() - window->f_center_screen());
	show_palette && available() && palette.hovered() && (!current_tool->is_active() || current_tool->type == ToolType::marquee) ? flags.set(GlobalFlags::palette_mode) : flags.reset(GlobalFlags::palette_mode);

}

void Editor::load() {
	map.load(*finder, finder->paths.room_name);
	palette.load(*finder, "palette", true);
	map.set_origin({});
	palette.set_origin({});
}

bool Editor::save() { return map.save(*finder, finder->paths.room_name); }

void Editor::render(sf::RenderWindow& win) {

	auto tileset = sf::Sprite{tileset_textures.at(map.get_i_style())};
	map.render(win, tileset);

	if (current_tool->in_bounds(map.dimensions) && !menu_hovered && !palette_mode() &&
		(current_tool->type == ToolType::brush || current_tool->type == ToolType::fill || current_tool->type == ToolType::erase || current_tool->type == ToolType::entity_editor)) {
		auto tileset = sf::Sprite{tileset_textures.at(map.get_i_style())};
		tileset.setTextureRect(sf::IntRect({palette.get_tile_coord(selected_block), {32, 32}}));
		for (int i = 0; i < current_tool->size; i++) {
			for (int j = 0; j < current_tool->size; j++) {
				target_shape.setPosition({(current_tool->f_scaled_position().x - i) * map.f_cell_size() + map.get_position().x, (current_tool->f_scaled_position().y - j) * map.f_cell_size() + map.get_position().y});
				target_shape.setSize({map.f_cell_size(), map.f_cell_size()});
				tileset.setPosition(target_shape.getPosition());
				tileset.setScale({map.get_scale(), map.get_scale()});
				if (current_tool->is_paintable()) { win.draw(tileset); }
				win.draw(target_shape);
			}
		}
	}

	if (show_palette) {
		palette.set_backdrop_color({40, 40, 40, 180});
		palette.render(win, tileset);
		if (palette_mode()) {
			selector.setSize({palette.f_cell_size(), palette.f_cell_size()});
			left_mouse_pressed() && palette_mode() ? selector.setOutlineColor({55, 255, 255, 180}) : selector.setOutlineColor({255, 255, 255, 80});
			right_mouse_pressed() && palette_mode() ? selector.setFillColor({50, 250, 250, 60}) : selector.setFillColor({50, 250, 250, 20});
			selector.setOutlineThickness(-2.f);
			selector.setPosition(palette.get_tile_position_at(static_cast<int>(current_tool->get_window_position().x - palette.get_position().x), static_cast<int>(current_tool->get_window_position().y - palette.get_position().y)) + palette.get_position());
			win.draw(selector);
		}
	}

	// render custom cursor
	auto tool = sf::Sprite{tool_texture};
	current_tool->render(map, win, map.get_position());
	current_tool->render(palette, win, palette.get_position());
	tool.setTextureRect({{static_cast<int>(current_tool->type) * 32, static_cast<int>(current_tool->status) * 32}, {32, 32}});
	tool.setPosition(current_tool->get_window_position());
	if (!ImGui::GetIO().MouseDrawCursor) { win.draw(tool); }

	// ImGui stuff
	gui_render(win);
}

void Editor::gui_render(sf::RenderWindow& win) {
	popup_open = false;
	bool* debug{};
	float const PAD = 10.0f;
	ImGuiIO& io = ImGui::GetIO();
	io.FontGlobalScale = 1.0;
	ImGuiViewport const* viewport = ImGui::GetMainViewport();
	ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar
	ImVec2 work_size = viewport->WorkSize;
	auto f_work_size = sf::Vector2<float>{static_cast<float>(work_size.x), static_cast<float>(work_size.y)};

	auto& target = palette_mode() ? palette : map;
	current_tool->set_position((sf::Vector2<float>{io.MousePos.x, io.MousePos.y} - target.get_position()) / target.get_scale());
	secondary_tool->set_position((sf::Vector2<float>{io.MousePos.x, io.MousePos.y} - target.get_position()) / target.get_scale());
	current_tool->set_window_position(sf::Vector2<float>{io.MousePos.x, io.MousePos.y});
	secondary_tool->set_window_position(sf::Vector2<float>{io.MousePos.x, io.MousePos.y});

	if (current_tool->type == ToolType::marquee && !window_hovered) {
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::Text("Press `Ctrl+X` to cut selection.");
		ImGui::Text("Press `Ctrl+C` to copy selection.");
		ImGui::Text("Press `Ctrl+V` to paste selection.");
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}

	if (current_tool->type == ToolType::entity_editor && !window_hovered) {
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::Text(current_tool->get_tooltip().c_str());
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}

	bool insp{};
	bool plat{};

	// Main Menu
	menu_hovered = false;
	if (ImGui::BeginMainMenuBar()) {
		bool new_popup{};
		bool save_popup{};
		bool save_as_popup{};
		if (ImGui::BeginMenu("File")) {
			menu_hovered = true;
			if (ImGui::MenuItem("New", NULL, &new_popup)) {}

			// Always center this window when appearing
			ImVec2 center(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);
			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			ImGui::Separator();
			if (ImGui::MenuItem("Open")) {
				popup_open = true;
				char filename[MAX_PATH];
				OPENFILENAME ofn;
				ZeroMemory(&filename, sizeof(filename));
				ZeroMemory(&ofn, sizeof(ofn));
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = NULL; // If you have a window to center over, put its HANDLE here
				ofn.lpstrFilter = "Json Files\0*.json\0Any File\0*.*\0Folders\0\0";
				ofn.lpstrFile = filename;
				ofn.nMaxFile = MAX_PATH;
				ofn.lpstrTitle = "Select a [meta.json] to load.";
				ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

				if (GetOpenFileNameA(&ofn)) {
					auto open_path = std::filesystem::path{filename};
					finder->paths.room_name = open_path.parent_path().filename().string();
					console.add_log(std::string{"filename: " + finder->paths.room_name}.c_str());
					load();
				} else {
					switch (CommDlgExtendedError()) {
					case CDERR_DIALOGFAILURE: std::cout << "CDERR_DIALOGFAILURE\n"; break;
					case CDERR_FINDRESFAILURE: std::cout << "CDERR_FINDRESFAILURE\n"; break;
					case CDERR_INITIALIZATION: std::cout << "CDERR_INITIALIZATION\n"; break;
					case CDERR_LOADRESFAILURE: std::cout << "CDERR_LOADRESFAILURE\n"; break;
					case CDERR_LOADSTRFAILURE: std::cout << "CDERR_LOADSTRFAILURE\n"; break;
					case CDERR_LOCKRESFAILURE: std::cout << "CDERR_LOCKRESFAILURE\n"; break;
					case CDERR_MEMALLOCFAILURE: std::cout << "CDERR_MEMALLOCFAILURE\n"; break;
					case CDERR_MEMLOCKFAILURE: std::cout << "CDERR_MEMLOCKFAILURE\n"; break;
					case CDERR_NOHINSTANCE: std::cout << "CDERR_NOHINSTANCE\n"; break;
					case CDERR_NOHOOK: std::cout << "CDERR_NOHOOK\n"; break;
					case CDERR_NOTEMPLATE: std::cout << "CDERR_NOTEMPLATE\n"; break;
					case CDERR_STRUCTSIZE: std::cout << "CDERR_STRUCTSIZE\n"; break;
					case FNERR_BUFFERTOOSMALL: std::cout << "FNERR_BUFFERTOOSMALL\n"; break;
					case FNERR_INVALIDFILENAME: std::cout << "FNERR_INVALIDFILENAME\n"; break;
					case FNERR_SUBCLASSFAILURE: std::cout << "FNERR_SUBCLASSFAILURE\n"; break;
					default: std::cout << "You cancelled.\n";
					}
				}
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Save", NULL, &save_popup)) {}
			if (ImGui::MenuItem("Save As", NULL, &save_as_popup)) {}
			ImGui::Separator();
			if (ImGui::MenuItem("Close", NULL)) { flags.set(GlobalFlags::shutdown); }
			ImGui::EndMenu();
		}
		if (save_popup) {
			if (save()) {
				console.add_log("File saved successfully.");
			} else {
				console.add_log("Encountered an error saving file!");
			}
		}
		if (new_popup) { ImGui::OpenPopup("New File"); }
		if (ImGui::BeginPopupModal("New File", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
			popup_open = true;
			ImGui::Text("Please enter a new room name.");
			ImGui::Text("Convention is all caps, snake-case, and of the format `REGION_ROOM_NUMBER`.");
			ImGui::Separator();
			ImGui::NewLine();
			static char buffer[128] = "";

			ImGui::InputTextWithHint("Folder Name", "DOJO_CORRIDOR_01", buffer, IM_ARRAYSIZE(buffer));
			ImGui::Separator();
			ImGui::NewLine();

			ImGui::Text("Please specify the dimensions of the level in chunks (16x16 tiles)");
			ImGui::Separator();
			ImGui::NewLine();

			static int width{1};
			static int height{1};
			static int room_id{0};
			static int metagrid_x{};
			static int metagrid_y{};

			width = std::clamp(width, 1, std::numeric_limits<int>::max());
			height = std::clamp(height, 1, std::numeric_limits<int>::max());

			ImGui::InputInt("Width", &width);
			ImGui::NewLine();

			ImGui::InputInt("Height", &height);
			ImGui::Separator();
			ImGui::NewLine();

			ImGui::InputInt("Room ID", &room_id);
			ImGui::Separator();
			ImGui::NewLine();

			ImGui::Text("Metagrid Position");
			ImGui::InputInt("X", &metagrid_x);
			ImGui::SameLine();

			ImGui::InputInt("Y", &metagrid_y);
			ImGui::Separator();
			ImGui::NewLine();

			if (ImGui::Button("Close")) { ImGui::CloseCurrentPopup(); }
			ImGui::SameLine();
			if (ImGui::Button("Create")) {

				static int style_current = static_cast<int>(map.styles.tile);
				static int bg_current = static_cast<int>(map.bg);

				map = Canvas({static_cast<uint32_t>(width * CHUNK_SIZE), static_cast<uint32_t>(height * CHUNK_SIZE)}, SelectionType::canvas);
				map.styles.tile = static_cast<Style>(style_current);
				map.bg = static_cast<Backdrop>(bg_current);
				map.metagrid_coordinates = {metagrid_x, metagrid_y};
				finder->paths.room_name = buffer;
				map.room_id = room_id;
				save();
				load();
				map.center(window->f_center_screen());
				std::string message = "Created new room with id " + std::to_string(room_id) + " and name " + finder->paths.room_name;
				console.add_log(message.data());
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		if (save_as_popup) { ImGui::OpenPopup("Save As"); }
		if (ImGui::BeginPopupModal("Save As", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
			popup_open = true;
			ImGui::Text("Please enter a unique room name.");
			ImGui::Text("Convention is all caps, snake-case, and of the format `Style_NAME_INDEX`.");
			ImGui::Separator();
			ImGui::NewLine();
			static char buffer[128] = "";

			ImGui::InputTextWithHint("Folder Name", "DOJO_CORRIDOR_01", buffer, IM_ARRAYSIZE(buffer));
			ImGui::Separator();
			ImGui::NewLine();

			if (ImGui::Button("Close")) { ImGui::CloseCurrentPopup(); }
			ImGui::SameLine();
			if (ImGui::Button("Create")) {
				finder->paths.room_name = buffer;
				save();
				ImGui::CloseCurrentPopup();
			}

			ImGui::TextUnformatted(buffer);

			ImGui::EndPopup();
		}
		if (ImGui::BeginMenu("Edit")) {
			menu_hovered = true;
			if (ImGui::MenuItem("Undo", "CTRL+Z")) { map.undo(); }
			if (ImGui::MenuItem("Redo", "CTRL+SHIFT+Z")) { map.redo(); }
			ImGui::EndMenu();
		}
		bool flag{};
		if (ImGui::BeginMenu("Insert")) {
			menu_hovered = true;
			if (ImGui::MenuItem("Inspectable", NULL, &insp)) {}
			if (ImGui::MenuItem("Platform", NULL, &plat)) {}
			if (ImGui::MenuItem("Save Point")) {
				current_tool = std::move(std::make_unique<EntityEditor>());
				current_tool->ent_type = EntityType::save_point;
			}
			if (ImGui::MenuItem("Player Placer")) {
				current_tool = std::move(std::make_unique<EntityEditor>());
				current_tool->ent_type = EntityType::player_placer;
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Tools")) {
			menu_hovered = true;
			if (ImGui::MenuItem("Brush", "B")) { current_tool = std::move(std::make_unique<Brush>()); }
			if (ImGui::MenuItem("Erase", "E")) { current_tool = std::move(std::make_unique<Erase>()); }
			if (ImGui::MenuItem("(-) size", "A")) { current_tool->change_size(-1); }
			if (ImGui::MenuItem("(+) size", "D")) { current_tool->change_size(1); }
			ImGui::Separator();
			if (ImGui::MenuItem("Hand", "H / Spacebar")) { current_tool = std::move(std::make_unique<Hand>()); }
			if (ImGui::MenuItem("Eyedropper", "Alt")) { current_tool = std::move(std::make_unique<Eyedropper>()); }
			if (ImGui::MenuItem("Fill", "G")) { current_tool = std::move(std::make_unique<Fill>()); }
			if (ImGui::MenuItem("Marquee", "M")) { current_tool = std::move(std::make_unique<Marquee>()); }
			if (ImGui::MenuItem("Entity Editor", "N")) { current_tool = std::move(std::make_unique<EntityEditor>()); }
			ImGui::Separator();
			if (ImGui::MenuItem("Center Map", "R")) { center_map(); }
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	std::string label{};
	if (insp) {
		ImGui::OpenPopup("Inspectable Message");
		label = "Inspectable Message";
		popup_open = true;
	}
	if (plat) {
		ImGui::OpenPopup("Platform Specifications");
		label = "Platform Specifications";
		popup_open = true;
	}
	popup.launch(*finder, console, label.c_str(), current_tool);

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

	ImVec2 window_pos;
	ImVec2 prev_window_pos{};
	ImVec2 prev_window_size{};
	window_pos.x = PAD;
	window_pos.y = work_pos.y + palette.dimensions.y * palette.f_cell_size() + 2 * PAD;
	window_flags |= ImGuiWindowFlags_NoMove;

	ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
	if (show_overlay) {
		ImGui::ShowDemoWindow();
		ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always);
		if (ImGui::Begin("Debug Mode", debug, window_flags)) {
			prev_window_size = ImGui::GetWindowSize();
			prev_window_pos = ImGui::GetWindowPos();
			ImGui::Text("Pioneer (beta version 1.0.0) - Level Editor");
			ImGui::Separator();
			if (ImGui::IsMousePosValid()) {
				ImGui::Text("Mouse Position: (%.1f,%.1f)", io.MousePos.x, io.MousePos.y);
			} else {
				ImGui::Text("Mouse Position: <invalid>");
			}
			ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
			if (ImGui::BeginTabBar("MyTabBar", tab_bar_flags)) {
				if (ImGui::BeginTabItem("General")) {
					ImGui::Text("Any Window Hovered: %s", window_hovered ? "Yes" : "No");
					ImGui::Text("Palette Mode: %s", palette_mode() ? "Yes" : "No");
					ImGui::Text("Has Palette Selection: %s", current_tool->has_palette_selection ? "Yes" : "No");
					ImGui::Separator();
					ImGui::Text("Zoom: %.2f", map.get_scale());
					ImGui::Separator();
					ImGui::Text("Room ID: %u", map.room_id);
					ImGui::Text("Tool Position: (%.1f,%.1f)", current_tool->f_position().x, current_tool->f_position().y);
					ImGui::Text("Map/Camera Position: (%.1f,%.1f)", map.get_position().x, map.get_position().y);
					ImGui::Text("Active Layer: %i", active_layer);
					ImGui::Text("Num Layers: %lu", map.get_layers().layers.size());
					ImGui::Text("Stored Tile Value: %u", current_tool->tile);
					if (current_tool->in_bounds(map.dimensions)) {
						ImGui::Text("Tile Value at Mouse Pos: %u", map.tile_val_at(current_tool->scaled_position().x, current_tool->scaled_position().y, active_layer));
						auto pos = map.get_tile_position_at(current_tool->scaled_position().x, current_tool->scaled_position().y, active_layer);
						ImGui::Text("Tile Position at Mouse Pos: (%.1f,%.1f)", pos.x, pos.y);
					} else {
						ImGui::Text("Tile Value at Mouse Pos: <invalid>");
					}
					ImGui::Separator();
					ImGui::Text("Current Style: %s", get_style_string.at(map.styles.tile));
					//ImGui::Text("Current Backdrop: %s", get_backdrop_string.at(map.bg));
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Tool")) {
					ImGui::Text("Left Mouse: %s", left_mouse_pressed() ? "Pressed" : "");
					ImGui::Text("Right Mouse: %s", right_mouse_pressed() ? "Pressed" : "");
					ImGui::Separator();
					static bool current{};
					ImGui::Checkbox("##current", &current);
					ImGui::SameLine();
					ImGui::Text("%s", current ? "Current" : "Secondary");
					auto& tool = current ? current_tool : secondary_tool;
					ImGui::Text("Tool Position: (%.1f,%.1f)", tool->f_position().x, tool->f_position().y);
					ImGui::Text("Tool Position (scaled): (%i,%i)", tool->scaled_position().x, tool->scaled_position().y);
					ImGui::Text("Tool Window Position: (%.1f,%.1f)", tool->get_window_position().x, tool->get_window_position().y);
					ImGui::Text("Tool in Bounds: %s", tool->in_bounds(map.dimensions) ? "Yes" : "No");
					ImGui::Text("Tool Ready: %s", tool->is_ready() ? "Yes" : "No");
					ImGui::Text("Tool Active: %s", tool->is_active() ? "Yes" : "No");
					ImGui::Text("Label: %s", get_tool_string.at(tool->type).c_str());
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Canvas")) {
					ImGui::Text("Map undo states: %i", map.undo_states_size());
					ImGui::Text("Map redo states: %i", map.redo_states_size());
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}

			prev_window_size = ImGui::GetWindowSize();
			ImGui::End();
		}
	}
	if (current_tool->type == ToolType::entity_editor) {
		ImGui::SetNextWindowBgAlpha(0.95f); // Transparent background
		work_size = viewport->WorkSize;
		window_pos = {window_pos.x, window_pos.y + prev_window_size.y + PAD};
		window_flags = ImGuiWindowFlags_NoCollapse;
		ImGui::SetNextWindowPos(window_pos);
		if (ImGui::Begin("Enemies", debug, window_flags)) {
			prev_window_size = ImGui::GetWindowSize();
			prev_window_pos = ImGui::GetWindowPos();
			int num_cols = 4;
			int num_rows = 4;
			for (int i = 0; i < num_rows; i++) {
				for (int j = 0; j < num_cols; j++) {
					auto idx = j + i * num_cols;
					ImGui::PushID(idx);
					if (ImGui::Button(std::to_string(idx).c_str())) {
						current_tool = std::move(std::make_unique<EntityEditor>());
						current_tool->ent_type = EntityType::critter;
					}
					ImGui::PopID();
					ImGui::SameLine();
				}
				ImGui::NewLine();
			}
			ImGui::End();
		}
		ImGui::SetNextWindowBgAlpha(0.95f); // Transparent background
		window_pos = {window_pos.x, window_pos.y + prev_window_size.y + PAD};
		window_flags = ImGuiWindowFlags_NoCollapse;
		ImGui::SetNextWindowPos(window_pos);
		if (ImGui::Begin("Entities", debug, window_flags)) {
			prev_window_size = ImGui::GetWindowSize();
			prev_window_pos = ImGui::GetWindowPos();

			if (ImGui::Button("Animator")) { ImGui::OpenPopup("Select Animator"); }
			if (ImGui::BeginPopupModal("Select Animator", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
				ImGui::Text("2x2:");
				int num_cols = 8;
				int num_rows = 2;
				for (int i = 0; i < num_rows; i++) {
					for (int j = 0; j < num_cols; j++) {
						ImGui::PushID(j + i * num_cols);
						auto idx = j + i * num_cols;
						//map.entities.sprites.large_animator.setTextureRect(sf::IntRect({{idx * 64, 0}, {64, 64}}));
						//map.entities.sprites.large_animator.setScale({0.5f, 0.5f});
						//if (ImGui::ImageButton(map.entities.sprites.large_animator, 2)) {
						//	current_tool = std::move(std::make_unique<EntityEditor>());
						//	current_tool->ent_type = EntityType::animator;
						//	//current_tool->current_animator = Animator{{.dimensions = sf::Vector2<uint32_t>{2u, 2u}, .id = idx + large_index_multiplier}, false, active_layer >= 4}; // change booleans here later
						//	ImGui::CloseCurrentPopup();
						//}
						ImGui::PopID();
						ImGui::SameLine();
					}
					ImGui::NewLine();
				}
				ImGui::Text("1x1:");
				num_cols = 16;
				num_rows = 2;
				for (int i = 0; i < num_rows; i++) {
					for (int j = 0; j < num_cols; j++) {
						ImGui::PushID(j + i * num_cols);
						auto idx = j + i * num_cols;
						//map.entities.sprites.small_animator.setTextureRect(sf::IntRect({{idx * 32, 0}, {32, 32}}));
						//if (ImGui::ImageButton(map.entities.sprites.small_animator, 2)) {
						//	current_tool = std::move(std::make_unique<EntityEditor>());
						//	current_tool->ent_type = EntityType::animator;
						//	//current_tool->current_animator = Animator{{.dimensions = sf::Vector2<uint32_t>{1u, 1u}, .id = idx + small_index_multiplier}, false, active_layer >= 4}; // change booleans here later
						//	ImGui::CloseCurrentPopup();
						//}
						ImGui::PopID();
						ImGui::SameLine();
					}
					ImGui::NewLine();
				}
				if (ImGui::Button("Close")) { ImGui::CloseCurrentPopup(); }
				ImGui::EndPopup();
			}
			if (ImGui::Button("Automatic Animators")) { ImGui::OpenPopup("Animator Specifications"); }
			if (ImGui::BeginPopupModal("Animator Specifications", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {

				static int id{};
				static int style{};
				static bool foreground{};
				ImGui::InputInt("Texture Lookup", &id);
				ImGui::InputInt("Style", &style);

				if (ImGui::Button("Create")) {
					// switch to entity tool, and store the specified portal for placement
					current_tool = std::move(std::make_unique<EntityEditor>());
					current_tool->ent_type = EntityType::animator;
					//current_tool->current_animator = Animator{{.dimensions = sf::Vector2<uint32_t>{1u, 1u}, .id = id}, true, false, style};
					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();
				if (ImGui::Button("Close")) { ImGui::CloseCurrentPopup(); }
				ImGui::EndPopup();
			}
			if (ImGui::Button("Portal")) { ImGui::OpenPopup("Portal Dimensions"); }
			if (ImGui::BeginPopupModal("Portal Dimensions", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {

				static int width{0};
				static int height{0};
				static int destination{0};
				static bool activate_on_contact{};
				static bool already_open{};
				static bool locked{};
				static int key_id{};

				ImGui::InputInt("Width", &width);
				ImGui::Separator();
				ImGui::NewLine();

				ImGui::InputInt("Height", &height);
				ImGui::Separator();
				ImGui::NewLine();

				ImGui::InputInt("Destination Room ID", &destination);
				ImGui::SameLine();
				help_marker("Must be an existing room before being activated in-game. By convention, choose a three-digit number where the first digit indicates the region.");
				ImGui::Separator();
				ImGui::NewLine();

				ImGui::Checkbox("Already open?", &already_open);
				ImGui::SameLine();
				help_marker("Only applies to 1x1 portals that are not activated on contact (doors). If true, the door will appear open.");
				ImGui::Separator();
				ImGui::NewLine();

				ImGui::Checkbox("Activate on contact?", &activate_on_contact);
				ImGui::SameLine();
				help_marker("If left unchecked, the player will have to inspect the portal to activate it.");
				ImGui::Separator();
				ImGui::NewLine();

				ImGui::Checkbox("Locked?", &locked);
				ImGui::SameLine();
				ImGui::InputInt("Key ID", &key_id);
				ImGui::Separator();
				ImGui::NewLine();

				if (ImGui::Button("Create")) {
					// switch to entity tool, and store the specified portal for placement
					current_tool = std::move(std::make_unique<EntityEditor>());
					current_tool->ent_type = EntityType::portal;
					//current_tool->current_portal = Portal{{.dimensions = sf::Vector2<uint32_t>{static_cast<uint32_t>(width), static_cast<uint32_t>(height)}}, activate_on_contact, already_open, static_cast<int>(map.room_id), destination, locked, key_id};
					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();
				if (ImGui::Button("Close")) { ImGui::CloseCurrentPopup(); }
				ImGui::EndPopup();
			}
			
			if (ImGui::Button("Chest")) { ImGui::OpenPopup("Chest Specifications"); }
			if (ImGui::BeginPopupModal("Chest Specifications", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {

				static int item_id{};
				static int id{};
				static int type{};
				static int amount{};
				static float rarity{};

				ImGui::SameLine();
				ImGui::Separator();
				ImGui::NewLine();

				ImGui::InputInt("Type", &type);
				ImGui::SameLine();
				help_marker("1 for gun, 2 for orbs, 3 for item");
				ImGui::Separator();
				ImGui::NewLine();

				ImGui::InputInt("Amount", &amount);
				ImGui::SameLine();
				help_marker("Number of orbs to drop. Only for type == 2.");
				ImGui::Separator();
				ImGui::NewLine();

				ImGui::InputFloat("Rarity", &rarity);
				ImGui::Separator();
				ImGui::NewLine();

				ImGui::InputInt("Item ID", &item_id);
				ImGui::SameLine();
				help_marker("For guns and items only.");
				ImGui::Separator();
				ImGui::NewLine();

				ImGui::InputInt("ID", &id);
				ImGui::SameLine();
				help_marker("Currently not used, just put a random number.");
				ImGui::Separator();
				ImGui::NewLine();

				if (ImGui::Button("Create")) {
					// switch to entity tool, and store the specified portal for placement
					current_tool = std::move(std::make_unique<EntityEditor>());
					current_tool->ent_type = EntityType::chest;
					//current_tool->current_chest = Chest{{.id = id}, item_id, type, rarity, amount};
					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();
				if (ImGui::Button("Close")) { ImGui::CloseCurrentPopup(); }
				ImGui::EndPopup();
			}
			if (ImGui::Button("Vine")) { ImGui::OpenPopup("Vine Specifications"); }
			if (ImGui::BeginPopupModal("Vine Specifications", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {

				static int length{};
				static int size{};
				static bool foreground{};
				static int type{};
				static int platform{};

				ImGui::SameLine();
				ImGui::Separator();
				ImGui::NewLine();

				ImGui::InputInt("Length", &length);
				ImGui::InputInt("Size", &size);
				ImGui::InputInt("Platform Indeces", &platform);
				ImGui::SameLine();
				help_marker("Either 1 (1x1) or 2 (2x2)");
				ImGui::Checkbox("Foreground?", &foreground);
				ImGui::InputInt("Type", &type);
				ImGui::SameLine();
				help_marker("0 : vine, 1 : grass");

				ImGui::NewLine();

				if (ImGui::Button("Create")) {
					// switch to entity tool, and store the specified portal for placement
					current_tool = std::move(std::make_unique<EntityEditor>());
					current_tool->ent_type = EntityType::interactive_scenery;
					//current_tool->current_interactive_scenery = InteractiveScenery{{}, length, size, foreground, type, platform != 0, {platform}};
					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();
				if (ImGui::Button("Close")) { ImGui::CloseCurrentPopup(); }
				ImGui::EndPopup();
			}
			if (ImGui::Button("Scenery")) { ImGui::OpenPopup("Scenery Specifications"); }
			if (ImGui::BeginPopupModal("Scenery Specifications", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {

				static int style{};
				static int layer{};
				static int variant{};

				ImGui::SameLine();
				ImGui::Separator();
				ImGui::NewLine();

				ImGui::InputInt("Style", &style);
				ImGui::InputInt("Layer", &layer);
				ImGui::InputInt("Variant", &variant);

				ImGui::NewLine();

				if (ImGui::Button("Create")) {
					// switch to entity tool, and store the specified portal for placement
					current_tool = std::move(std::make_unique<EntityEditor>());
					current_tool->ent_type = EntityType::scenery;
					//current_tool->current_scenery = Scenery{{}, style, layer, variant};
					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();
				if (ImGui::Button("Close")) { ImGui::CloseCurrentPopup(); }
				ImGui::EndPopup();
			}
			if (ImGui::Button("Switch")) { ImGui::OpenPopup("Switch Specifications"); }
			if (ImGui::BeginPopupModal("Switch Specifications", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {

				static int button_id{};
				static int type{};

				ImGui::SameLine();
				ImGui::Separator();
				ImGui::NewLine();
				
				ImGui::InputInt("Type", &type);
				ImGui::SameLine();
				help_marker("0 for toggler, 1 for permanent, 2 for movable");
				ImGui::Separator();
				ImGui::NewLine();

				ImGui::InputInt("ID", &button_id);
				ImGui::SameLine();
				help_marker("To match with blocks");
				ImGui::Separator();
				ImGui::NewLine();

				if (ImGui::Button("Create")) {
					// switch to entity tool, and store the specified portal for placement
					current_tool = std::move(std::make_unique<EntityEditor>());
					current_tool->ent_type = EntityType::switch_button;
					current_tool->current_entity = std::make_unique<SwitchButton>(button_id, type);

					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();
				if (ImGui::Button("Close")) { ImGui::CloseCurrentPopup(); }
				ImGui::EndPopup();
			}
			if (ImGui::Button("Switch Block")) { ImGui::OpenPopup("Switch Block Specifications"); }
			if (ImGui::BeginPopupModal("Switch Block Specifications", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {

				static int button_id{};
				static int type{};

				ImGui::SameLine();
				ImGui::Separator();
				ImGui::NewLine();

				ImGui::InputInt("Type", &type);
				ImGui::SameLine();
				help_marker("0 for toggler, 1 for permanent, 2 for movable");
				ImGui::Separator();
				ImGui::NewLine();

				ImGui::InputInt("ID", &button_id);
				ImGui::SameLine();
				help_marker("To match with blocks");
				ImGui::Separator();
				ImGui::NewLine();

				if (ImGui::Button("Create")) {
					// switch to entity tool, and store the specified portal for placement
					current_tool = std::move(std::make_unique<EntityEditor>());
					current_tool->ent_type = EntityType::switch_block;
					//current_tool->current_switch_block = SwitchBlock{{.id = button_id}, type};
					ImGui::CloseCurrentPopup();
				}
				ImGui::SameLine();
				if (ImGui::Button("Close")) { ImGui::CloseCurrentPopup(); }
				ImGui::EndPopup();
			}
			ImGui::End();
		}
	}

	ImGui::SetNextWindowBgAlpha(0.65f); // Transparent background
	work_pos = viewport->WorkPos;		// Use work area to avoid menu-bar/task-bar, if any!
	work_size = viewport->WorkSize;
	window_pos.x = work_pos.x + work_size.x - PAD;
	window_pos.y = work_pos.y + PAD;
	window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
	ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, {1, 0});
	if (ImGui::Begin("Settings", debug, window_flags)) {
		window_flags = ImGuiWindowFlags_None;
		window_flags |= ImGuiWindowFlags_MenuBar;
		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
		ImGui::BeginChild("ChildR", ImVec2(320, 72), true, window_flags);
		ImGui::BeginMenuBar();
		if (ImGui::BeginMenu("Tools")) { ImGui::EndMenu(); }

		ImGui::EndMenuBar();
		static auto tools = sf::Sprite{tool_texture};
		for (int i = 0; i < NUM_TOOLS; i++) {
			ImGui::PushID(i); 
			tools.setTextureRect(sf::IntRect{{i * 32, 0}, {32, 32}});
			ImGui::ImageButton(std::to_string(i).c_str(), tools, {32.f, 32.f}, sf::Color::Transparent, sf::Color::White);
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
				ImGui::Text(get_tool_string.at((ToolType)i).c_str());
				ImGui::PopTextWrapPos();
				ImGui::EndTooltip();
			}
			if (ImGui::IsItemClicked()) {
				switch ((ToolType)i) {
				case ToolType::brush: current_tool = std::move(std::make_unique<Brush>()); break;
				case ToolType::fill: current_tool = std::move(std::make_unique<Fill>()); break;
				case ToolType::marquee: current_tool = std::move(std::make_unique<Marquee>()); break;
				case ToolType::erase: current_tool = std::move(std::make_unique<Erase>()); break;
				case ToolType::hand: current_tool = std::move(std::make_unique<Hand>()); break;
				case ToolType::entity_editor: current_tool = std::move(std::make_unique<EntityEditor>()); break;
				default: current_tool = std::move(std::make_unique<Hand>()); break;
				}
			}
			ImGui::PopID();
			ImGui::SameLine();
		}

		ImGui::EndChild();
		ImGui::PopStyleVar();

		ImGui::Separator();
		ImGui::Text("Brush Size");
		ImGui::SliderInt("##brushsz", &current_tool->size, 1, 16);
		if (current_tool->type == ToolType::marquee) {
			if (ImGui::Checkbox("Pervasive", &tool_flags.pervasive)) { current_tool->clear(); }
			help_marker("If checked, actions will apply to all layers.");
		} else if (current_tool->type == ToolType::fill) {
			if (ImGui::Checkbox("Pervasive", &tool_flags.pervasive)) {}
			help_marker("If checked, actions will apply to all layers.");
			if (ImGui::Checkbox("Contiguous", &tool_flags.contiguous)) {}
			help_marker("If checked, actions will only apply to connected sections.");
		} else {
			ImGui::NewLine();
		}
		ImGui::Separator();
		ImGui::Text("Current Block:");
		auto tileset = sf::Sprite{tileset_textures.at(map.get_i_style())};
		tileset.setTextureRect(sf::IntRect({palette.get_tile_coord(selected_block), {32, 32}}));
		ImGui::Image(tileset);
		if (current_tool->type == ToolType::entity_editor) {
			ImGui::Text("Current Entity:");
			switch (current_tool->ent_type) {
			case EntityType::portal: ImGui::Text("Portal"); break;
			case EntityType::inspectable: ImGui::Text("Inspectable"); break;
			case EntityType::animator: ImGui::Text("Animator"); break;
			case EntityType::critter: ImGui::Text("Critter"); break;
			case EntityType::player_placer: ImGui::Text("Player Placer"); break;
			case EntityType::save_point: ImGui::Text("Save Point"); break;
			case EntityType::chest: ImGui::Text("Chest"); break;
			default: ImGui::Text("<None>"); break;
			}
		}
		if (current_tool->in_bounds(map.dimensions)) {
			ImGui::Text("Tool Position : (%i,%i)", current_tool->scaled_position().x, current_tool->scaled_position().y);
		} else {
			ImGui::Text("Tool Position : ---");
		}

		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
		ImGui::BeginChild("ChildS", ImVec2(320, 172), true, window_flags);
		ImGui::BeginMenuBar();
		if (ImGui::BeginMenu("Current Layer")) { ImGui::EndMenu(); }
		ImGui::EndMenuBar();
		for (int i = 0; i < 8; ++i) {
			if (ImGui::Selectable(layer_name.at(i), active_layer == i)) { active_layer = i; }
		}
		ImGui::EndChild();
		ImGui::PopStyleVar();

		ImGui::Separator();
		ImGui::Text("Canvas Settings");
		ImGui::Separator();

		if(ImGui::BeginTabBar("##gensettings")) {
			if (ImGui::BeginTabItem("General")) {
				ImGui::Checkbox("Debug Overlay", &show_overlay);
				ImGui::Checkbox("Show Entities", &map.flags.show_entities);
				ImGui::Checkbox("Show Background", &map.flags.show_background);
				ImGui::Checkbox("Show Grid", &map.flags.show_grid);
				ImGui::Checkbox("Show Palette", &show_palette);
				ImGui::EndTabItem();
			}
			if(ImGui::BeginTabItem("Layer")) {
				ImGui::Checkbox("Show All Layers", &map.flags.show_all_layers);
				ImGui::Checkbox("Show Obscured Layer", &map.flags.show_obscured_layer);
				ImGui::Checkbox("Show Indicated Layers", &map.flags.show_indicated_layers);
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}

		ImGui::Separator();
		ImGui::Text("Actions");
		ImGui::Separator();
		if (ImGui::Button("Center Canvas")) { map.center(window->f_center_screen()); }
		ImGui::SameLine();
		if (ImGui::Button("Reset Zoom")) { map.set_scale(1.f); }
		ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.5, 0.6f, 0.5f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.5, 0.7f, 0.6f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.5, 0.8f, 0.7f));
		static bool save_before{true};
		ImGui::Checkbox("##savebefore", &save_before);
		ImGui::SameLine();
		help_marker("Save changes when launching demo");
		ImGui::SameLine();
		if (ImGui::Button("Launch Demo")) {
			trigger_demo = true;
			if (save_before) { save(); }
		};
		ImGui::PopStyleColor(3);
		if (ImGui::Button("Export Layer to .png")) {
			screencap.resize({win.getSize().x, win.getSize().y});
			export_layer_texture();
		};
		if (ImGui::Button("Undo")) { map.undo(); }
		ImGui::SameLine();
		if (ImGui::Button("Redo")) { map.redo(); }

		ImGui::Text("Scene Style: ");
		int style_current = static_cast<int>(map.styles.tile);
		if (ImGui::Combo("##scenestyle", &style_current, styles, IM_ARRAYSIZE(styles))) { map.styles.tile = static_cast<Style>(style_current); }
		ImGui::Text("Scene Background: ");
		int bg_current = static_cast<int>(map.bg);
		if (ImGui::Combo("##scenebg", &bg_current, bgs, IM_ARRAYSIZE(bgs))) { map.bg = static_cast<Backdrop>(bg_current); }

		// resize map
		ImGui::Separator();
		ImGui::Text("Map width:");
		ImGui::SameLine();
		if (ImGui::ArrowButton("##minus_width", ImGuiDir_Left)) { map.resize({-1, 0}); }
		ImGui::SameLine();
		if (ImGui::ArrowButton("##plus_width", ImGuiDir_Right)) { map.resize({1, 0}); }
		ImGui::Text("Map height:");
		ImGui::SameLine();
		if (ImGui::ArrowButton("##minus_height", ImGuiDir_Up)) { map.resize({0, -1}); }
		ImGui::SameLine();
		if (ImGui::ArrowButton("##plus_height", ImGuiDir_Down)) { map.resize({0, 1}); }
		ImGui::Separator();

		// destructive actions
		if (ImGui::Button("Clear Layer")) {
			map.save_state(*current_tool, true);
			map.get_layers().layers.at(active_layer).clear();
		}
		if (ImGui::Button("Clear All Layers")) {
			map.save_state(*current_tool, true);
			for (auto& layer : map.get_layers().layers) { layer.clear(); }
		}
		if (ImGui::Button("Clear Entire Canvas")) {
			map.save_state(*current_tool, true);
			for (auto& layer : map.get_layers().layers) { layer.clear(); }
			map.entities.clear();
		}
		ImGui::Separator();
		prev_window_size = ImGui::GetWindowSize();
		prev_window_pos = ImGui::GetWindowPos();
		ImGui::End();
	}
	console.write_console(prev_window_size, prev_window_pos);
}

void Editor::help_marker(char const* desc) {
	ImGui::SameLine();
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered()) {
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

void Editor::export_layer_texture() {
	screencap.clear(sf::Color::Transparent);
	sf::Vector2u mapdim{map.get_real_dimensions()};
	screencap.resize({mapdim.x, mapdim.y});
	for (int i = 0; i <= active_layer; ++i) {
		for (auto& cell : map.get_layers().layers.at(i).grid.cells) {
			if (cell.value > 0) {
				auto x_coord = (cell.value % 16) * TILE_WIDTH;
				auto y_coord = std::floor(cell.value / 16) * TILE_WIDTH;
				auto tile_sprite = sf::Sprite{tileset_textures.at(static_cast<int>(map.styles.tile))};
				tile_sprite.setTextureRect(sf::IntRect({static_cast<int>(x_coord), static_cast<int>(y_coord)}, {32, 32}));
				tile_sprite.setPosition(cell.scaled_position());
				screencap.draw(tile_sprite);
			}
		}
	}
	std::time_t const now = std::time(nullptr);
	std::string filedate = std::asctime(std::localtime(&now));
	std::erase_if(filedate, [](auto const& c) { return c == ':' || isspace(c); });
	std::string filename = "screenshot_" + filedate + ".png";
	if (screencap.getTexture().copyToImage().saveToFile(filename)) {
		std::string log = "Screenshot saved to " + filename + "\n";
		console.add_log(log.data());
	}
}

void Editor::center_map() {
	map.center(window->f_center_screen());
	map.set_scale(1.f);
}

void Editor::launch_demo(char** argv, int room_id, std::filesystem::path path, sf::Vector2<float> player_position) {
	trigger_demo = false;
	ImGui::SFML::Shutdown();
	fornani::Application demo{argv};
	std::cout << "Editor path: " << path.string() << "\n";
	std::cout << "AAA room id: " << room_id << std::endl;
	std::cout << "\n\n" << finder->paths.room_name << "\n\n";
	demo.launch(argv, true, room_id, finder->paths.room_name, player_position);
}

} // namespace pi
