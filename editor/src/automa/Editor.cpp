
#include "fornani/setup/ResourceFinder.hpp"
#include "fornani/core/Application.hpp"
#include "editor/util/Lookup.hpp"
#include "editor/automa/Editor.hpp"
#include "editor/gui/Console.hpp"
#include "editor/canvas/entity/SavePoint.hpp"

#include <filesystem>
#include <algorithm>

namespace pi {

Editor::Editor(char** argv, WindowManager& window, fornani::data::ResourceFinder& finder)
	: window(&window), finder(&finder), map(finder, SelectionType::canvas), palette(finder, SelectionType::palette), current_tool(std::make_unique<Hand>()), secondary_tool(std::make_unique<Hand>()), grid_refresh(16), active_layer{0} {

	args = argv;
	std::cout << "Level path: " << finder.paths.levels << "\n";
	if (!tool_texture.loadFromFile((finder.paths.editor / "gui" / "tools.png").string())) { std::cout << "Failed to load tool texture.\n"; }

	for (int i = 0; i < static_cast<int>(StyleType::END); ++i) {
		m_themes.styles.push_back(Style{static_cast<StyleType>(i)});
		m_labels.style_str[i] = m_themes.styles.back().get_label();
		m_labels.styles[i] = m_labels.style_str[i].c_str();
	}
	for (int i = 0; i < static_cast<int>(Backdrop::END); ++i) {
		m_themes.backdrops.push_back(BackgroundType{static_cast<Backdrop>(i)});
		m_labels.bg_str[i] = m_themes.backdrops.back().get_label();
		m_labels.backdrops[i] = m_labels.bg_str[i].c_str();
	}

	user_data = dj::Json::from_file((finder.paths.editor / "data" / "config" / "user.json").string().c_str());
	assert(!user_data.is_null());
	console.add_log("Welcome to Pioneer!");
	finder.paths.region = "config";
	finder.paths.room_name = "new_file.json";
	finder.paths.region = user_data["region"].as_string();
	finder.paths.room_name = user_data["room"].as_string();
	std::string msg = "Loading room: <" + finder.region_and_room().string() + ">";
	console.add_log(msg.data());
	load();
	map.activate_middleground();
	palette.flags.show_entities = false;

	target_shape.setFillColor(sf::Color{110, 90, 200, 80});
	target_shape.setOutlineColor(sf::Color{240, 230, 255, 100});
	target_shape.setOutlineThickness(-2);
	target_shape.setSize({map.f_cell_size(), map.f_cell_size()});

	center_map();
}

void Editor::run() {

	// load the tilesets!
	for (int i = 0; i < static_cast<int>(StyleType::END); ++i) {
		tileset_textures.push_back(sf::Texture());
		std::string style = Style{static_cast<StyleType>(i)}.get_label();
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

		if (flags.test(GlobalFlags::shutdown)) {
			shutdown(*finder);
			return;
		}

		if (trigger_demo) {
			auto ppos = shift_pressed() ? sf::Vector2<float>{map.entities.variables.player_hot_start} * 32.f : sf::Vector2<float>{map.entities.variables.player_start} * 32.f;
			launch_demo(args, map.room_id, finder->paths.room_name, ppos);
			if (!ImGui::SFML::Init(window->get())) { console.add_log("ImGui::SFML::Init() failed!\n"); };
			init(finder->paths.room_name);
		}

		// events
		while (std::optional const event = window->get().pollEvent()) {
			ImGui::SFML::ProcessEvent(window->get(), *event);
			if (event->is<sf::Event::Closed>()) {
				shutdown(*finder);
				return;
			}
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
		if (!menu_hovered && !popup_open && !control_pressed()) {
			if (key_pressed->scancode == sf::Keyboard::Scancode::A) { current_tool->change_size(-1); }
			if (key_pressed->scancode == sf::Keyboard::Scancode::D) { current_tool->change_size(1); }
			if (key_pressed->scancode == sf::Keyboard::Scancode::R) { center_map(); }
			if (key_pressed->scancode == sf::Keyboard::Scancode::H) { current_tool = std::move(std::make_unique<Hand>()); }
			if (key_pressed->scancode == sf::Keyboard::Scancode::B) { current_tool = std::move(std::make_unique<Brush>()); }
			if (key_pressed->scancode == sf::Keyboard::Scancode::G) { current_tool = std::move(std::make_unique<Fill>()); }
			if (key_pressed->scancode == sf::Keyboard::Scancode::E) { current_tool = std::move(std::make_unique<Erase>()); }
			if (key_pressed->scancode == sf::Keyboard::Scancode::M) { current_tool = std::move(std::make_unique<Marquee>()); }
			if (key_pressed->scancode == sf::Keyboard::Scancode::N) { current_tool = std::move(std::make_unique<EntityEditor>()); }
			if (key_pressed->scancode == sf::Keyboard::Scancode::Escape) { m_clipboard = {}; }
			if (key_pressed->scancode == sf::Keyboard::Scancode::Tab) { map.flags.show_grid = !map.flags.show_grid; }
		}
		if(shift_pressed() && !control_pressed()) {
			if (key_pressed->scancode == sf::Keyboard::Scancode::Up) { active_layer = std::clamp(active_layer - 1, 0, static_cast<int>(map.get_layers().layers.size())); }
			if (key_pressed->scancode == sf::Keyboard::Scancode::Down) { active_layer = std::clamp(active_layer + 1, 0, static_cast<int>(map.get_layers().layers.size())); }
		}
		if (control_pressed()) {
			if (key_pressed->scancode == sf::Keyboard::Scancode::X) {
				current_tool->handle_keyboard_events(source, key_pressed->scancode);
				if (current_tool->selection) {
					m_clipboard = Clipboard(current_tool->selection.value().dimensions);
					m_clipboard.value().cut(source, *current_tool);
				}
			}
			if (key_pressed->scancode == sf::Keyboard::Scancode::C) {
				current_tool->handle_keyboard_events(source, key_pressed->scancode);
				if (current_tool->selection) {
					m_clipboard = Clipboard(current_tool->selection.value().dimensions);
					m_clipboard.value().copy(source, *current_tool);
				}
			}
			if (key_pressed->scancode == sf::Keyboard::Scancode::V && !palette_mode()) {
				current_tool->handle_keyboard_events(map, key_pressed->scancode);
				if (m_clipboard) { m_clipboard.value().paste(map, *current_tool); }
			}
			if (key_pressed->scancode == sf::Keyboard::Scancode::D) { m_clipboard = {}; }
			if (key_pressed->scancode == sf::Keyboard::Scancode::L) {
				save();
				trigger_demo = true;
			}
			if (key_pressed->scancode == sf::Keyboard::Scancode::S) { save() ? console.add_log("File saved successfully.") : console.add_log("Encountered an error saving file!"); }
			if (shift_pressed()) {
				if (key_pressed->scancode == sf::Keyboard::Scancode::L) {
					map.entities.variables.player_hot_start = current_tool->scaled_position();
					save();
					trigger_demo = true;
				}
				if (key_pressed->scancode == sf::Keyboard::Scancode::Left) { map.resize({-1, 0}); }
				if (key_pressed->scancode == sf::Keyboard::Scancode::Right) { map.resize({1, 0}); }
				if (key_pressed->scancode == sf::Keyboard::Scancode::Up) { map.resize({0, -1}); }
				if (key_pressed->scancode == sf::Keyboard::Scancode::Down) { map.resize({0, 1}); }
			}
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
			grid_refresh.start();
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

	auto& target = palette_mode() ? palette : map;
	auto& tool = right_mouse_pressed() ? secondary_tool : current_tool;
	map.constrain(window->f_screen_dimensions());
	m_middleground = map.get_layers().get_middleground();

	window_hovered = ImGui::IsAnyItemHovered() || ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow) || ImGui::IsAnyItemActive();
	current_tool->palette_mode = palette_mode();

	// tool logic
	if (available() && !palette_mode()) { map.save_state(*tool); }

	left_mouse_pressed() && current_tool->is_ready() && available() ? current_tool->activate() : current_tool->deactivate();
	right_mouse_pressed() && secondary_tool->is_ready() && available() ? secondary_tool->activate() : secondary_tool->deactivate();
	tool->update(target);

	if ((any_mouse_pressed()) && !menu_hovered && !window_hovered) {
		if (tool->type == ToolType::eyedropper) { selected_block = current_tool->tile; }
		if (palette_mode() && current_tool->type != ToolType::marquee) {
			auto pos = current_tool->get_window_position() - palette.get_position();
			auto idx = palette.tile_val_at_scaled(static_cast<int>(pos.x), static_cast<int>(pos.y), 0);
			current_tool->store_tile(idx);
			selected_block = idx;
			if (!current_tool->is_paintable()) {
				current_tool = std::move(std::make_unique<Brush>());
				current_tool->suppress_until_released();
			}
		}
	}

	palette.active_layer = 0;
	map.active_layer = active_layer;
	if (current_tool->trigger_switch) { current_tool = std::move(std::make_unique<Hand>()); }
	current_tool->tile = selected_block;
	current_tool->pervasive = tool_flags.pervasive;
	current_tool->contiguous = tool_flags.contiguous;
	current_tool->set_usability(current_tool->in_bounds(target.dimensions));

	map.update(*current_tool);
	palette.update(*current_tool);
	if (window_hovered || popup_open || menu_hovered) {
		map.unhover();
		palette.unhover();
	}
	palette.set_position({12.f, 32.f});
	if (palette.hovered()) { map.unhover(); }

	map.set_offset_from_center(map.get_position() + map.get_scaled_center() - window->f_center_screen());
	m_options.palette && available() && palette.hovered() && (!current_tool->is_active() || current_tool->type == ToolType::marquee) ? flags.set(GlobalFlags::palette_mode) : flags.reset(GlobalFlags::palette_mode);

	grid_refresh.update();
	if (grid_refresh.is_almost_complete()) { map.set_grid_texture(); }

	map.flags.show_all_layers = shift_pressed() ? map.flags.show_current_layer : !map.flags.show_current_layer;
	map.flags.show_current_layer = shift_pressed() ? map.flags.show_all_layers : !map.flags.show_all_layers;

	// set tool positions
	ImGuiIO& io = ImGui::GetIO();
	current_tool->set_position((sf::Vector2<float>{io.MousePos.x, io.MousePos.y} - target.get_position()) / target.get_scale());
	secondary_tool->set_position((sf::Vector2<float>{io.MousePos.x, io.MousePos.y} - target.get_position()) / target.get_scale());
	current_tool->set_window_position(sf::Vector2<float>{io.MousePos.x, io.MousePos.y});
	secondary_tool->set_window_position(sf::Vector2<float>{io.MousePos.x, io.MousePos.y});
}

void Editor::load() {
	map.load(*finder, finder->paths.region, finder->paths.room_name);
	palette.load(*finder, "palette", "palette.json", true);
	map.set_origin({});
	palette.set_origin({});
	reset_layers();
}

bool Editor::save() { return map.save(*finder, finder->paths.region, finder->paths.room_name); }

void Editor::render(sf::RenderWindow& win) {

	auto tileset = sf::Sprite{tileset_textures.at(map.get_i_style())};
	map.render(win, tileset);

	auto soft_palette_mode = m_options.palette && available() && palette.hovered();
	if (current_tool->in_bounds(map.dimensions) && !menu_hovered && !palette_mode() &&
		current_tool->highlight_canvas() && !soft_palette_mode) {
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

	if (m_clipboard && (control_pressed() || current_tool->type == ToolType::marquee) && !current_tool->is_active()) { m_clipboard.value().render(map, *current_tool, win, map.get_position()); }

	if (m_options.palette) {
		palette.hovered() ? palette.set_backdrop_color({90, 90, 90, 255}) : palette.set_backdrop_color({40, 40, 40, 180});
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

	if(current_tool->entity_menu) {
		if (current_tool->current_entity) { ImGui::OpenPopup("Entity Options"); }
	}
	if (current_tool->entity_mode == EntityMode::editor) {
		if (current_tool->current_entity) { ImGui::OpenPopup("Edit Entity"); }
	}
	if (ImGui::BeginPopupContextWindow("Entity Options")) {
		if (ImGui::MenuItem("Edit")) {
			current_tool->entity_mode = EntityMode::editor;
			current_tool->entity_menu = false;
		}
		if (ImGui::MenuItem("Move")) {
			current_tool->entity_mode = EntityMode::mover;
			current_tool->entity_menu = false;
		}
		if (ImGui::MenuItem("Duplicate")) {
			current_tool->entity_mode = EntityMode::placer;
			current_tool->entity_menu = false;
		}
		if (ImGui::MenuItem("Delete")) {
			current_tool->entity_mode = EntityMode::eraser;
			current_tool->entity_menu = false;
		}
		ImGui::EndPopup();
	}
	if (ImGui::BeginPopupContextWindow("Edit Entity")) {
		if (current_tool->current_entity) {
			current_tool->current_entity.value()->expose();
			if (ImGui::Button("Save Changes")) {
				for (auto& ent : map.entities.variables.entities) {
					if (ent->highlighted) { ent->overwrite = true; }
				}
				current_tool->suppress_until_released();
				ImGui::CloseCurrentPopup();
			}
		}
		ImGui::EndPopup();
	}

	bool insp{};
	bool plat{};
	bool port{};
	bool enem{};
	bool open_themes{};

	// Main Menu
	menu_hovered = false;
	if (ImGui::BeginMainMenuBar()) {
		bool new_popup{};
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
				ofn.lpstrTitle = "Select a .json file to load.";
				ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

				if (GetOpenFileNameA(&ofn)) {
					auto open_path = std::filesystem::path{filename};
					finder->paths.region = open_path.parent_path().filename().string();
					finder->paths.room_name = open_path.filename().string();
					console.add_log(std::string{"region: " + finder->paths.region}.c_str());
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
			if (ImGui::MenuItem("Save", "Ctrl+S")) { save() ? console.add_log("File saved successfully.") : console.add_log("Encountered an error saving file!"); }
			if (ImGui::MenuItem("Save As", NULL, &save_as_popup)) {}
			if (ImGui::MenuItem("Close", NULL)) { flags.set(GlobalFlags::shutdown); }
			ImGui::EndMenu();
		}
		if (new_popup) { ImGui::OpenPopup("New File"); }
		if (ImGui::BeginPopupModal("New File", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
			popup_open = true;
			ImGui::Text("Please enter a new room name.");
			ImGui::Text("Convention is all lowercase, snake-case, and of the format `room_name`.");
			ImGui::Separator();
			ImGui::NewLine();
			static char regbuffer[128] = "";
			static char roombuffer[128] = "";

			ImGui::InputTextWithHint("Region Name", "firstwind", regbuffer, IM_ARRAYSIZE(regbuffer));
			ImGui::InputTextWithHint("Room Name", "boiler_room", roombuffer, IM_ARRAYSIZE(roombuffer));
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

				static int style_current = static_cast<int>(map.styles.tile.get_type());
				static int bg_current = static_cast<int>(map.background->type.get_type());

				map = Canvas(*finder, {static_cast<uint32_t>(width * chunk_size_v), static_cast<uint32_t>(height * chunk_size_v)}, SelectionType::canvas, static_cast<StyleType>(style_current), static_cast<Backdrop>(bg_current));
				map.metagrid_coordinates = {metagrid_x, metagrid_y};
				finder->paths.region = regbuffer;
				finder->paths.room_name = std::string{roombuffer} + ".json";
				map.room_id = room_id;
				save();
				load();
				reset_layers();
				map.center(window->f_center_screen());
				console.add_log(std::string{"In folder " + finder->paths.region}.c_str());
				console.add_log(std::string{"Created new room with id " + std::to_string(room_id) + " and name " + finder->paths.room_name}.c_str());
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		if (save_as_popup) { ImGui::OpenPopup("Save As"); }
		if (ImGui::BeginPopupModal("Save As", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
			popup_open = true;
			ImGui::Text("Please enter a new room name.");
			ImGui::Text("Convention is all lowercase, snake-case, and of the format `room_name`.");
			ImGui::Separator();
			ImGui::NewLine();
			static char regbuffer[128] = "";
			static char roombuffer[128] = "";

			ImGui::InputTextWithHint("Region Name", "firstwind", regbuffer, IM_ARRAYSIZE(regbuffer));
			ImGui::InputTextWithHint("Room Name", "boiler_room", roombuffer, IM_ARRAYSIZE(roombuffer));
			ImGui::Separator();
			ImGui::NewLine();

			if (ImGui::Button("Close")) { ImGui::CloseCurrentPopup(); }
			ImGui::SameLine();
			if (ImGui::Button("Create")) {
				finder->paths.region = regbuffer;
				finder->paths.room_name = std::string{roombuffer} + ".json";
				save();
				ImGui::CloseCurrentPopup();
			}

			ImGui::TextUnformatted(regbuffer);
			ImGui::TextUnformatted(roombuffer);

			ImGui::EndPopup();
		}
		if (ImGui::BeginMenu("Edit")) {
			menu_hovered = true;
			if (ImGui::MenuItem("Undo", "Ctrl+Z")) { map.undo(); }
			if (ImGui::MenuItem("Redo", "Ctrl+Shift+Z")) { map.redo(); }
			ImGui::Separator();
			if (ImGui::MenuItem("Clear Clipboard", "Ctrl+D")) { m_clipboard = {}; }
			ImGui::Separator();
			if (ImGui::MenuItem("(+) Map Width", "Ctrl+Shift+RightArrow")) { map.resize({1, 0}); }
			if (ImGui::MenuItem("(-) Map Width", "Ctrl+Shift+LeftArrow")) { map.resize({-1, 0}); }
			if (ImGui::MenuItem("(+) Map Height", "Ctrl+Shift+DownArrow")) { map.resize({0, 1}); }
			if (ImGui::MenuItem("(-) Map Height", "Ctrl+Shift+UpArrow")) { map.resize({0, -1}); }
			ImGui::Separator();
			if (ImGui::MenuItem("Clear Layer")) {
				map.save_state(*current_tool, true);
				map.get_layers().layers.at(active_layer).clear();
			}
			if (ImGui::MenuItem("Clear All Layers")) {
				map.save_state(*current_tool, true);
				for (auto& layer : map.get_layers().layers) { layer.clear(); }
			}
			if (ImGui::MenuItem("Clear Entire Canvas")) {
				map.save_state(*current_tool, true);
				for (auto& layer : map.get_layers().layers) { layer.clear(); }
				map.entities.clear();
			}
			ImGui::Separator();
			if (ImGui::BeginMenu("Style")) {
				auto i{0};
				for (auto& choice : m_labels.styles) {
					if (ImGui::MenuItem(choice)) { map.styles.tile = Style{static_cast<StyleType>(i)}; }
					++i;
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Background")) {
				auto i{0};
				for (auto& choice : m_labels.backdrops) {
					if (ImGui::MenuItem(choice)) { map.background = std::make_unique<Background>(*finder, static_cast<Backdrop>(i)); }
					++i;
				}
				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("Include in Minimap", "", &map.minimap)) {}
			if (ImGui::MenuItem("Themes", "", &open_themes)) {}
			
			ImGui::EndMenu();
		}
		bool flag{};
		if (ImGui::BeginMenu("Insert")) {
			menu_hovered = true;
			if (ImGui::MenuItem("Portal", NULL, &port)) {}
			if (ImGui::MenuItem("Inspectable", NULL, &insp)) {}
			if (ImGui::MenuItem("Platform", NULL, &plat)) {}
			if (ImGui::MenuItem("Enemy", NULL, &enem)) {}
			if (ImGui::MenuItem("Save Point")) {
				current_tool = std::move(std::make_unique<EntityEditor>(EntityMode::placer));
				current_tool->current_entity = std::make_unique<SavePoint>(map.room_id);
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Player Placer")) {
				current_tool = std::move(std::make_unique<EntityEditor>(EntityMode::placer));
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
			if (ImGui::MenuItem("Hand", "H")) { current_tool = std::move(std::make_unique<Hand>()); }
			if (ImGui::MenuItem("Eyedropper", "Alt")) { current_tool = std::move(std::make_unique<Eyedropper>()); }
			if (ImGui::MenuItem("Fill", "G")) { current_tool = std::move(std::make_unique<Fill>()); }
			if (ImGui::MenuItem("Marquee", "M")) { current_tool = std::move(std::make_unique<Marquee>()); }
			if (ImGui::MenuItem("Entity Editor", "N")) { current_tool = std::move(std::make_unique<EntityEditor>()); }
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Actions")) {
			menu_hovered = true;
			if (ImGui::MenuItem("Export Layer to .png")) { export_layer_texture(); }
			ImGui::Separator();
			if (ImGui::MenuItem("Demo fullscreen", "", &m_demo.fullscreen)) {}
			if (ImGui::MenuItem("Save and Launch Demo", "Ctrl+L")) {
				save();
				trigger_demo = true;
			}
			if (ImGui::MenuItem("Launch Demo without Saving")) { trigger_demo = true; }
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View")) {
			menu_hovered = true;
			if (ImGui::MenuItem("Center Canvas", "R")) { center_map(); }
			ImGui::Separator();
			ImGui::MenuItem("Show Sidebar", "", &m_options.sidebar);
			ImGui::MenuItem("Show Console", "", &m_options.console);
			ImGui::MenuItem("Show Palette", "", &m_options.palette);
			ImGui::Separator();
			ImGui::Checkbox("Debug Overlay", &show_overlay);
			ImGui::Checkbox("Show Entities", &map.flags.show_entities);
			ImGui::Checkbox("Show Background", &map.flags.show_background);
			ImGui::Checkbox("Show Grid", &map.flags.show_grid);
			ImGui::EndMenu();
		}
		ImGui::EndMainMenuBar();
	}

	if (open_themes) { ImGui::OpenPopup("Level Themes"); }
	if (ImGui::BeginPopupModal("Level Themes", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		static char musbuffer[128] = "";
		static char ambbuffer[128] = "";
		ImGui::InputTextWithHint("Music", "whispering_island", musbuffer, IM_ARRAYSIZE(musbuffer));
		ImGui::InputTextWithHint("Ambience", "overturned", ambbuffer, IM_ARRAYSIZE(ambbuffer));
		if (ImGui::Button("Apply")) {
			if (std::string{musbuffer}.size() > 0) { map.m_theme.music = musbuffer; }
			if (std::string{ambbuffer}.size() > 0) { map.m_theme.ambience = ambbuffer; }
			ImGui::CloseCurrentPopup(); }
		if (ImGui::Button("Close")) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
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
	if (port) {
		ImGui::OpenPopup("Portal Specifications");
		label = "Portal Specifications";
		popup_open = true;
	}
	if (enem) {
		ImGui::OpenPopup("Enemy Specifications");
		label = "Enemy Specifications";
		popup_open = true;
	}

	popup.launch(*finder, console, label.c_str(), current_tool, map.room_id);

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

	ImVec2 window_pos;
	ImVec2 prev_window_pos{};
	ImVec2 prev_window_size{};
	window_pos.x = work_pos.x + palette.dimensions.x * palette.f_cell_size() + 2 * PAD;
	window_pos.y = PAD;
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
					ImGui::Text("paths/resources: %s", finder->paths.resources.string().c_str());
					ImGui::Text("paths/editor...: %s", finder->paths.editor.string().c_str());
					ImGui::Text("paths/levels...: %s", finder->paths.levels.string().c_str());
					ImGui::Text("region: %s", finder->paths.region.c_str());
					ImGui::Text("room..: %s", finder->paths.room_name.c_str());
					ImGui::Separator();
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
					ImGui::Text("Current Style: %s", map.styles.tile.get_label().c_str());
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
					ImGui::Text("Tool Window Position (scaled): (%.1f,%.1f)", tool->get_window_position_scaled().x, tool->get_window_position_scaled().y);
					ImGui::Text("Tool in Bounds: %s", tool->in_bounds(map.dimensions) ? "Yes" : "No");
					ImGui::Text("Tool Ready: %s", tool->is_ready() ? "Yes" : "No");
					ImGui::Text("Tool Active: %s", tool->is_active() ? "Yes" : "No");
					ImGui::Text("Label: %s", tool->get_label().c_str());
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Canvas")) {
					ImGui::Text("Map hovered? %s", map.hovered() ? "Yes" : "No");
					ImGui::Text("Palette hovered? %s", palette.hovered() ? "Yes" : "No");
					ImGui::Text("Map undo states: %i", map.undo_states_size());
					ImGui::Text("Map redo states: %i", map.redo_states_size());
					ImGui::Separator();
					ImGui::Text("Number of Layers: %i", map.get_layers().layers.size());
					ImGui::Text("Middleground: %i", map.get_layers().get_middleground());
					ImGui::Text("Active Layer: %i", active_layer);
					for (auto& layer : map.get_layers().layers) { ImGui::Text("Layer: %i", layer.render_order); }
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}

			prev_window_size = ImGui::GetWindowSize();
			ImGui::End();
		}
	}

	if (m_options.sidebar) {
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
					ImGui::Text(current_tool->get_label().c_str());
					ImGui::PopTextWrapPos();
					ImGui::EndTooltip();
				}
				if (ImGui::IsItemClicked()) {
					switch (static_cast<ToolType>(i)) {
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
				if (ImGui::Checkbox("Pervasive", &tool_flags.pervasive)) {}
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
				if (current_tool->current_entity) {
					ImGui::Text("Current Entity: %s", current_tool->current_entity.value()->get_label().c_str());
				} else {
					ImGui::Text("Current Entity: <None>");
				}
			}
			if (current_tool->in_bounds(map.dimensions)) {
				ImGui::Text("Tool Position : (%i,%i)", current_tool->scaled_position().x, current_tool->scaled_position().y);
			} else {
				ImGui::Text("Tool Position : ---");
			}
			ImGui::Separator();
			ImGui::Text("Middleground: ");
			ImGui::SameLine();
			if (ImGui::InputInt("##smg", &m_middleground)) {
				m_middleground = std::clamp(m_middleground, 0, static_cast<int>(map.get_layers().layers.size()) - 1);
				map.get_layers().set_middleground(m_middleground);
				reset_layers();
			}
			ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
			ImGui::BeginChild("ChildS", ImVec2(320, 172), true, window_flags);
			ImGui::BeginMenuBar();
			if (ImGui::BeginMenu("Current Layer")) {
				if (ImGui::MenuItem("Insert Layer in Front")) {
					map.get_layers().add_layer(active_layer, 1);
					reset_layers();
				}
				if (ImGui::MenuItem("Insert Layer Behind")) {
					map.get_layers().add_layer(active_layer, 0);
					reset_layers();
				}
				if (ImGui::MenuItem("Delete Current Layer")) { delete_current_layer(); }
				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
			auto ctr{0};
			for (auto& layer : map.get_layers().layers) {
				if (ImGui::Selectable(m_labels.layers[ctr], active_layer == ctr)) { active_layer = ctr; }
				++ctr;
			}
			ImGui::EndChild();
			ImGui::PopStyleVar();

			ImGui::Separator();
			ImGui::Text("Canvas Settings");
			ImGui::Separator();

			if (ImGui::BeginTabBar("##gensettings")) {
				if (ImGui::BeginTabItem("General")) {
					ImGui::Checkbox("Debug Overlay", &show_overlay);
					ImGui::Checkbox("Show Entities", &map.flags.show_entities);
					ImGui::Checkbox("Show Background", &map.flags.show_background);
					ImGui::Checkbox("Show Grid", &map.flags.show_grid);
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Layer")) {
					if (ImGui::Checkbox("Show All Layers", &map.flags.show_all_layers)) { map.flags.show_current_layer = !map.flags.show_all_layers; };
					ImGui::Checkbox("Show Obscured Layer", &map.flags.show_obscured_layer);
					ImGui::Checkbox("Show Indicated Layers", &map.flags.show_indicated_layers);
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}

			prev_window_size = ImGui::GetWindowSize();
			prev_window_pos = ImGui::GetWindowPos();
			ImGui::End();
		}
		if (m_options.console) { console.write_console(prev_window_size, prev_window_pos); }
	}

	if (current_tool->type == ToolType::entity_editor && !window_hovered && current_tool->entity_mode != EntityMode::editor && !current_tool->entity_menu) {
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::Text(current_tool->get_tooltip().c_str());
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
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
	if (!screencap.resize({mapdim.x, mapdim.y})) { console.add_log("Export to .png failed!"); };
	for (int i = 0; i <= active_layer; ++i) {
		for (auto& cell : map.get_layers().layers.at(i).grid.cells) {
			if (cell.value > 0) {
				auto x_coord = (cell.value % 16) * TILE_WIDTH;
				auto y_coord = std::floor(cell.value / 16) * TILE_WIDTH;
				auto tile_sprite = sf::Sprite{tileset_textures.at(static_cast<int>(map.styles.tile.get_type()))};
				tile_sprite.setTextureRect(sf::IntRect({static_cast<int>(x_coord), static_cast<int>(y_coord)}, {32, 32}));
				tile_sprite.setPosition(cell.scaled_position());
				screencap.draw(tile_sprite);
			}
		}
	}
	std::time_t time = std::time({});
	char time_string[std::size("yyyy-mm-ddThh:mm:ssZ")];
	std::strftime(std::data(time_string), std::size(time_string), "%FT%TZ", std::gmtime(&time));
	std::string time_str{time_string};

	std::erase_if(time_str, [](auto const& c) { return c == ':' || isspace(c); });
	std::string filename = "screenshot_" + time_str + ".png";
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
	pressed_keys = {};
	current_tool->current_entity = {};
	ImGui::SFML::Shutdown();
	fornani::Application demo{argv};
	std::cout << "> Launching Demo\n";
	std::cout << "Editor path: " << path.string() << "\n";
	std::cout << "Room ID: " << room_id << std::endl;
	std::cout << "Room Name: " << finder->paths.room_name << "\n";
	demo.init(argv, {true, m_demo.fullscreen});
	demo.launch(argv, true, room_id, finder->paths.room_name, player_position);
}

void Editor::shutdown(fornani::data::ResourceFinder& finder) {
	user_data["region"] = finder.paths.region;
	user_data["room"] = finder.paths.room_name;
	if (!user_data.to_file((finder.paths.editor / "data" / "config" / "user.json").string().c_str())) { std::cout << "Failed to save user data.\n"; }
}

void Editor::reset_layers() {
	map.get_layers().set_labels();
	for (int i = 0; i < static_cast<int>(map.get_layers().layers.size()); ++i) {
		m_labels.layer_str[i] = map.get_layers().get_layer_name(i);
		m_labels.layers[i] = m_labels.layer_str[i].c_str();
	}
}

void Editor::delete_current_layer() {
	auto& layers = map.get_layers().layers;
	if (layers.size() <= 1) {
		console.add_log("Cannot delete only layer.");
		return;
	}
	map.save_state(*current_tool, true);
	map.get_layers().delete_layer_at(active_layer);
	reset_layers();
	if (layers.size() <= 1) { return; }
	active_layer = std::clamp(active_layer, 0, static_cast<int>(layers.size()) - 1);
}

} // namespace pi
