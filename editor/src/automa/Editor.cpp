
#include "editor/automa/Editor.hpp"
#include <editor/automa/EditorContext.hpp>
#include <editor/util/Constants.hpp>
#include <fornani/events/SystemEvent.hpp>
#include <algorithm>
#include <cstring>
#include "editor/gui/Console.hpp"
#include "fornani/core/Application.hpp"
#include "fornani/setup/ResourceFinder.hpp"

#ifdef _WIN32
// TODO: debloat include
#include <Windows.h>
#endif

#include <filesystem>

namespace pi {

static bool b_reloaded{};

Editor::Editor(fornani::automa::ServiceProvider& svc, EditorContext& ctx)
	: EditorState(svc, ctx), map(svc, SelectionType::canvas, fornani::Biome{}), palette(svc, SelectionType::palette, fornani::Biome{}), current_tool(std::make_unique<Hand>()), secondary_tool(std::make_unique<Hand>()), grid_refresh(16),
	  active_layer{0}, m_tool_sprite{svc.assets.get_texture("editor_tools")}, m_services(&svc), m_menu_alpha{.5f} {

	p_target_state = EditorStateType::editor;

	svc.window->get().requestFocus();
	svc.window->get().setVisible(true);

	svc.events.new_file_event.attach_to(p_slot, &Editor::new_file, this);
	svc.events.load_file_event.attach_to(p_slot, &Editor::load_file, this);

	svc.set_editor(true);

	console.add_log("Welcome to Pioneer!");
	std::string msg = "Loading room: <" + p_services->finder.region_and_room().string() + ">";
	console.add_log(msg.data());
	load();
	console.add_log(std::string{"Room ID: " + std::to_string(map.room_id)}.data());
	map.activate_middleground();
	palette.flags.show_entities = false;

	target_shape.setFillColor(sf::Color{110, 90, 200, 80});
	target_shape.setOutlineColor(sf::Color{240, 230, 255, 100});
	target_shape.setOutlineThickness(-2);
	target_shape.setSize({map.f_cell_size(), map.f_cell_size()});

	center_map();

	// load the tilesets!
	for (auto const& biome : svc.data.biomes["biomes"].as_array()) {
		tileset_textures.push_back(sf::Texture());
		std::string filename = biome.as_string() + "_tiles.png";
		if (!tileset_textures.back().loadFromFile((p_services->finder.paths.resources / "image" / "tile" / filename).string())) { console.add_log(std::string{"Failed to load " + filename}.c_str()); }
	}

	bool debug_mode = false;

	p_wallpaper.setSize(p_services->window->f_screen_dimensions());
	p_colors.backdrop = sf::Color{40, 60, 80};

	m_overlay.setSize(svc.window->get_f_display_dimensions());
	m_overlay.setFillColor({60, 60, 60, 160});
}

EditorStateType Editor::run(char** argv) {

	if (m_demo.trigger.is_almost_complete()) {
		p_alt.reset();
		p_control.reset();
		p_shift.reset();
		auto ppos = m_demo.custom_position ? sf::Vector2f{map.entities.variables.player_hot_start} * 32.f : sf::Vector2f{map.entities.variables.player_start} * 32.f;
		launch_demo(argv, map.room_id, p_services->finder.paths.room_name, ppos);
		if (!ImGui::SFML::Init(p_services->window->get())) { console.add_log("ImGui::SFML::Init() failed!\n"); };
	}
	m_demo.trigger.update();

	logic();

	ImGuiIO& io = ImGui::GetIO();
	io.MouseDrawCursor = io.WantCaptureMouse;
	p_services->window->get().setMouseCursorVisible(false);

	EditorState::render(p_services->window->get());
	render(p_services->window->get());
	ImGui::SFML::Render(p_services->window->get());
	p_services->window->get().display();

	if (p_target_state != EditorStateType::editor) { save(); }
	EditorState::run(argv);
	return p_target_state;
}

void Editor::handle_events(std::optional<sf::Event> const event, sf::RenderWindow& win) {
	EditorState::handle_events(event, win);
	ImGuiIO& io = ImGui::GetIO();
	auto& source = palette_mode() || current_tool->has_palette_selection ? palette : map;

	// keyboard events
	if (!io.WantCaptureKeyboard) {
		if (auto const* key_pressed = event->getIf<sf::Event::KeyPressed>()) {
			if (!key_pressed->control) {
				if (key_pressed->scancode == sf::Keyboard::Scancode::A) { current_tool->change_size(-1); }
				if (key_pressed->scancode == sf::Keyboard::Scancode::D) { current_tool->change_size(1); }
				if (key_pressed->scancode == sf::Keyboard::Scancode::R) { center_map(); }
				if (key_pressed->scancode == sf::Keyboard::Scancode::T) {
					current_tool->set_mode(BrushMode::tile);
					m_options.palette = true;
				}
				if (key_pressed->scancode == sf::Keyboard::Scancode::H) { current_tool->set_mode(BrushMode::hazard); }
				if (key_pressed->scancode == sf::Keyboard::Scancode::B) { current_tool = std::move(std::make_unique<Brush>()); }
				if (key_pressed->scancode == sf::Keyboard::Scancode::F) { current_tool = std::move(std::make_unique<Fill>()); }
				if (key_pressed->scancode == sf::Keyboard::Scancode::E) { current_tool = std::move(std::make_unique<Erase>()); }
				if (key_pressed->scancode == sf::Keyboard::Scancode::M) { current_tool = std::move(std::make_unique<Marquee>()); }
				if (key_pressed->scancode == sf::Keyboard::Scancode::N) { current_tool = std::move(std::make_unique<EntityEditor>()); }
				if (key_pressed->scancode == sf::Keyboard::Scancode::Escape) { m_clipboard = {}; }
				if (key_pressed->scancode == sf::Keyboard::Scancode::Tab) { map.flags.show_grid = !map.flags.show_grid; }
			}
			if (key_pressed->shift && !key_pressed->control) {
				if (key_pressed->scancode == sf::Keyboard::Scancode::Up) { active_layer = std::clamp(active_layer > 0 ? active_layer - 1 : std::size_t{0}, std::size_t{0}, map.get_layers().layers.size() - 1); }
				if (key_pressed->scancode == sf::Keyboard::Scancode::Down) { active_layer = std::clamp(active_layer < map.get_layers().layers.size() - 1 ? active_layer + 1 : std::size_t{0}, std::size_t{0}, map.get_layers().layers.size()); }
			}
			if (key_pressed->control) {
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
				if (key_pressed->scancode == sf::Keyboard::Scancode::L) {
					save();
					m_demo.trigger.start();
					if (key_pressed->alt) { m_demo.fullscreen = true; }
				}
				if (key_pressed->scancode == sf::Keyboard::Scancode::S) { save() ? console.add_log("File saved successfully.") : console.add_log("Encountered an error saving file!"); }
				if (key_pressed->shift) {
					if (key_pressed->scancode == sf::Keyboard::Scancode::L) {
						map.entities.variables.player_hot_start = current_tool->scaled_position();
						save();
						m_demo.trigger.start();
						m_demo.custom_position = true;
						if (key_pressed->alt) { m_demo.fullscreen = true; }
					}
					if (key_pressed->scancode == sf::Keyboard::Scancode::Left) { map.resize({-1, 0}); }
					if (key_pressed->scancode == sf::Keyboard::Scancode::Right) { map.resize({1, 0}); }
					if (key_pressed->scancode == sf::Keyboard::Scancode::Up) { map.resize({0, -1}); }
					if (key_pressed->scancode == sf::Keyboard::Scancode::Down) { map.resize({0, 1}); }
				}
			}
			if (key_pressed->scancode == sf::Keyboard::Scancode::Q) { current_tool->handle_keyboard_events(map, key_pressed->scancode); }
			if (key_pressed->scancode == sf::Keyboard::Scancode::Z) {
				if (key_pressed->control && !key_pressed->shift) { map.undo(); }
				if (key_pressed->control && key_pressed->shift) { map.redo(); }
			}
		}
	}

	// zoom controls
	if (!io.WantCaptureKeyboard && !io.WantCaptureMouse) {
		if (auto const* scrolled = event->getIf<sf::Event::MouseWheelScrolled>()) {
			if (control_held()) {
				current_tool->direction.rotate(scrolled->delta > 0 ? fornani::RotationType::counterclockwise : fornani::RotationType::clockwise);
			} else {
				auto delta = scrolled->delta * zoom_factor * map.get_scale();
				if (map.within_zoom_limits(delta)) { map.move(current_tool->f_position() * -delta); }
				map.zoom(delta);
				grid_refresh.start();
			}
		}
	}
}

void Editor::logic() {
	EditorState::logic();
	// input results

	if (p_left_mouse.released) {
		current_tool->unsuppress();
		current_tool->release();
		current_tool->neutralize();
	}
	if (p_right_mouse.released) {
		secondary_tool->unsuppress();
		secondary_tool->release();
		secondary_tool->neutralize();
	}
	if (left_mouse_clicked()) { current_tool->click(); }
	if (right_mouse_clicked()) { secondary_tool->click(); }

	auto& target = palette_mode() ? palette : map;
	auto& tool = p_right_mouse.held ? secondary_tool : current_tool;
	map.constrain(p_services->window->f_screen_dimensions());
	m_middleground = map.get_layers().get_middleground();

	current_tool->set_mode(m_mode);

	ImGuiIO& io = ImGui::GetIO();
	map.set_state(CanvasState::available, !io.WantCaptureMouse);
	current_tool->palette_mode = palette_mode();

	if (tool->type == ToolType::entity_editor) { map.flags.show_entities = true; }

	// tool logic
	if (!is_any_widget_hovered() && !palette_mode()) { map.save_state(*tool); }
	if (!hazard_hovered) {
		left_mouse_held() && current_tool->is_ready() ? current_tool->activate() : current_tool->deactivate();
		right_mouse_held() && secondary_tool->is_ready() ? secondary_tool->activate() : secondary_tool->deactivate();
	}
	tool->update(target);

	if (left_mouse_clicked()) {
		if (tool->type == ToolType::eyedropper) {
			selected_block = current_tool->tile;
		} else if (palette_mode() && current_tool->type != ToolType::marquee) {
			if (!current_tool->is_paintable()) { current_tool = std::move(std::make_unique<Brush>()); }
			auto pos = current_tool->get_window_position() - palette.get_position();
			auto idx = palette.tile_val_at_scaled(static_cast<int>(pos.x), static_cast<int>(pos.y), 0);
			current_tool->store_tile(idx);
			selected_block = idx;
			current_tool->suppress_until_released();
		}
	}
	if (p_alt.clicked) {
		if (current_tool->type == ToolType::brush || current_tool->type == ToolType::erase) { current_tool = std::move(std::make_unique<Eyedropper>()); }
	}
	if (p_alt.released) {
		if (current_tool->type == ToolType::eyedropper) { current_tool = std::move(std::make_unique<Brush>()); }
	}

	if (hazard_mode()) { m_options.palette = false; }

	palette.active_layer = 0;
	map.active_layer = active_layer;
	if (current_tool->trigger_switch) { current_tool = std::move(std::make_unique<Hand>()); }
	current_tool->tile = selected_block;
	current_tool->pervasive = tool_flags.pervasive;
	current_tool->contiguous = tool_flags.contiguous;
	current_tool->set_usability(current_tool->in_bounds(target.dimensions) || palette_mode());

	map.update(*current_tool);
	palette.update(*current_tool);
	if (io.WantCaptureMouse) {
		map.unhover();
		palette.unhover();
	}
	palette.set_position({12.f, 32.f});
	if (palette.hovered() && m_options.palette) { map.unhover(); }

	map.set_offset_from_center(map.get_position() + map.get_scaled_center() - p_services->window->f_center_screen());
	m_options.palette && !io.WantCaptureMouse && palette.hovered() && (!current_tool->is_active() || current_tool->type == ToolType::marquee) ? flags.set(GlobalFlags::palette_mode) : flags.reset(GlobalFlags::palette_mode);

	grid_refresh.update();
	if (grid_refresh.is_almost_complete()) { map.set_grid_texture(); }

	map.flags.show_all_layers = !shift_held() || control_held();

	// set tool positions
	current_tool->set_position((p_current_mouse_position - target.get_position()) / target.get_scale());
	secondary_tool->set_position((p_current_mouse_position - target.get_position()) / target.get_scale());
	current_tool->set_window_position(p_current_mouse_position);
	secondary_tool->set_window_position(p_current_mouse_position);
}

void Editor::load() {
	if (!map.load(*p_services, p_services->finder, p_services->finder.paths.region, p_services->finder.paths.room_name)) { console.add_log("Encountered an error loading file!"); }
	if (!palette.load(*p_services, p_services->finder, "palette", "palette.json", true)) { console.add_log("Encountered an error loading palette!"); }
	map.set_origin({});
	palette.set_origin({});
	reset_layers();
	b_reloaded = true;
}

void Editor::load_file(std::string_view to_region, std::string_view to_room) {
	save();
	p_services->finder.paths.region = to_region;
	p_services->finder.paths.room_name = to_room;
	load();
	NANI_LOG_INFO(p_logger, "Loaded file: {}.", to_room);
}

void Editor::new_file(int id) {
	editor_flags.set(EditorFlags::create_new_room);
	p_new_id = id;
}

bool Editor::save() {
	auto ret = map.save(p_services->finder, p_services->finder.paths.region, p_services->finder.paths.room_name);
	auto path = p_services->finder.paths.levels / p_services->finder.paths.region / p_services->finder.paths.room_name;
	auto room_data_result = dj::Json::from_file(path.string());
	if (!room_data_result) {
		NANI_LOG_ERROR(p_logger, "Failed to reload saved JSON data after serialization. PATH: {}.", path.string());
		return false;
	}
	if (auto existing_data = p_services->data.get_map_json_from_id(map.room_id)) {
		existing_data->get() = std::move(*room_data_result);
		NANI_LOG_INFO(p_logger, "Re-serialized existing room. PATH: {}.", path.string());
	}

	return ret;
}

void Editor::render(sf::RenderWindow& win) {
	auto tileset = sf::Sprite{tileset_textures.at(map.get_i_style())};
	map.render(win, tileset);

	auto& io = ImGui::GetIO();
	auto mouse_position = sf::Vector2f{io.MousePos.x, io.MousePos.y};

	if (current_tool->in_bounds(map.dimensions) && !palette_mode() && current_tool->highlight_canvas() && !is_any_widget_hovered()) {
		auto tileset = current_tool->is_mode(BrushMode::tile) ? sf::Sprite{tileset_textures.at(map.get_i_style())} : sf::Sprite{m_services->assets.get_texture("hazard_" + std::string{map.get_hazard_properties().tag})};
		auto dim = current_tool->is_mode(BrushMode::tile) ? fornani::constants::i_resolution_vec : map.get_hazard_properties().dimensions;
		auto lookup = current_tool->is_mode(BrushMode::tile) ? palette.get_tile_coord(selected_block) : map.get_hazard_properties().get_lookup(selected_block);
		tileset.setTextureRect(sf::IntRect({lookup, dim}));
		for (int i = 0; i < current_tool->size; i++) {
			for (int j = 0; j < current_tool->size; j++) {
				target_shape.setPosition({(current_tool->f_scaled_position().x - i) * map.f_cell_size() + map.get_position().x, (current_tool->f_scaled_position().y - j) * map.f_cell_size() + map.get_position().y});
				target_shape.setSize({map.f_cell_size(), map.f_cell_size()});
				tileset.setPosition(target_shape.getPosition());
				tileset.setScale(map.get_scale_vec());
				if (current_tool->is_mode(BrushMode::hazard)) {
					tileset.setOrigin(sf::Vector2f{map.get_hazard_properties().dimensions} / 2.f);
					tileset.setRotation(current_tool->direction.as_angle());
					tileset.setPosition(target_shape.getPosition() + sf::Vector2f{map.get_hazard_properties().dimensions / 32} * map.f_cell_size());
				}
				if (current_tool->is_paintable()) { win.draw(tileset); }
				win.draw(target_shape);
			}
		}
	}

	if (hazard_mode()) {
		auto hpos = sf::Vector2f{8.f, 28.f};
		auto hazard_map = sf::Sprite{m_services->assets.get_texture("hazard_" + std::string{map.get_hazard_properties().tag})};
		auto hbox = sf::RectangleShape{};
		hbox.setSize(hazard_map.getLocalBounds().size);
		hbox.setPosition(hpos);
		hazard_hovered = hbox.getGlobalBounds().contains(mouse_position);
		hazard_hovered ? hbox.setOutlineThickness(4.f) : hbox.setOutlineThickness(2.f);
		hazard_hovered ? hbox.setOutlineColor(fornani::colors::white) : hbox.setOutlineColor(fornani::colors::blue);
		hazard_hovered ? hbox.setFillColor(sf::Color{127, 127, 127, 200}) : hbox.setFillColor(sf::Color{127, 127, 127, 40});
		hazard_map.setPosition(hpos);
		win.draw(hbox);
		win.draw(hazard_map);

		auto hazard_mouse_pos = mouse_position - hazard_map.getPosition();
		auto htile = sf::Vector2i{hazard_mouse_pos.componentWiseDiv(sf::Vector2f{map.get_hazard_properties().dimensions})};
		if (left_mouse_clicked() && current_tool->type == ToolType::brush) {
			selected_block = htile.y * map.get_hazard_properties().table_dimensions.x + htile.x;
			current_tool->store_tile(selected_block);
		}

		if (hazard_hovered) {
			auto tbox = sf::RectangleShape{};
			tbox.setFillColor(sf::Color::Transparent);
			tbox.setSize(sf::Vector2f{map.get_hazard_properties().dimensions});
			tbox.setPosition(hpos + sf::Vector2f{htile.componentWiseMul(map.get_hazard_properties().dimensions)});
			tbox.setOutlineThickness(-2.f);
			tbox.setOutlineColor(fornani::colors::mythic_green);
			win.draw(tbox);
		}
	} else {
		hazard_hovered = false;
	}

	if (m_clipboard && (control_held() || current_tool->type == ToolType::marquee) && !current_tool->is_active()) { m_clipboard.value().render(map, *current_tool, win, map.get_position()); }

	if (m_options.palette) {
		palette.hovered() ? palette.set_backdrop_color({90, 90, 90, 255}) : palette.set_backdrop_color({40, 40, 40, 180});
		palette.render(win, tileset);
		if (palette_mode()) {
			selector.setSize({palette.f_cell_size(), palette.f_cell_size()});
			left_mouse_clicked() && palette_mode() ? selector.setOutlineColor({55, 255, 255, 180}) : selector.setOutlineColor({255, 255, 255, 80});
			right_mouse_clicked() && palette_mode() ? selector.setFillColor({50, 250, 250, 60}) : selector.setFillColor({50, 250, 250, 20});
			selector.setOutlineThickness(-2.f);
			selector.setPosition(palette.get_tile_position_at(static_cast<int>(current_tool->get_window_position().x - palette.get_position().x), static_cast<int>(current_tool->get_window_position().y - palette.get_position().y)) +
								 palette.get_position());
			win.draw(selector);
		}
	}

	// render custom cursor
	current_tool->render(map, win, map.get_position());
	current_tool->render(palette, win, palette.get_position());
	m_tool_sprite.setTextureRect({{static_cast<int>(current_tool->type) * constants::tool_size_v, static_cast<int>(current_tool->status) * constants::tool_size_v}, {constants::tool_size_v, constants::tool_size_v}});
	m_tool_sprite.setScale(sf::Vector2f{constants::ui_tool_scale_v, constants::ui_tool_scale_v});
	m_tool_sprite.setPosition(current_tool->get_window_position());
	if (!ImGui::GetIO().MouseDrawCursor) { win.draw(m_tool_sprite); }

	// ImGui stuff
	gui_render(win);

	if (m_demo.trigger.running()) { win.draw(m_overlay); }
}

void Editor::gui_render(sf::RenderWindow& win) {
	bool* debug{};
	float const PAD = 10.0f;
	ImGuiIO& io = ImGui::GetIO();
	io.FontGlobalScale = 1.0;
	ImGuiViewport const* viewport = ImGui::GetMainViewport();
	ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar
	ImVec2 work_size = viewport->WorkSize;
	auto f_work_size = sf::Vector2f{static_cast<float>(work_size.x), static_cast<float>(work_size.y)};

	if (current_tool->entity_menu) {
		if (current_tool->current_entity) {
			ImGui::OpenPopup("Entity Options");
		} else {
			ImGui::OpenPopup("Non-Copyable Entity Options");
		}
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
	if (ImGui::BeginPopupContextWindow("Non-Copyable Entity Options")) {
		if (ImGui::MenuItem("Delete")) {
			current_tool->entity_mode = EntityMode::eraser;
			current_tool->entity_menu = false;
		}
		ImGui::EndPopup();
		ImGui::EndPopup();
	}
	if (ImGui::BeginPopupContextWindow("Edit Entity")) {
		if (current_tool->current_entity) {
			current_tool->current_entity.value()->expose();
			if (ImGui::Button("Save Changes") || editor_flags.test(EditorFlags::create_new_room)) {
				for (auto& ent : map.entities.variables.entities) {
					if (ent->highlighted) { ent->overwrite = true; }
					ent->selected = false;
					ent->highlighted = false;
				}
				ImGui::CloseCurrentPopup();
			}
			if (ImGui::Button("Close")) {
				for (auto& ent : map.entities.variables.entities) {
					ent->selected = false;
					ent->highlighted = false;
				}
				current_tool->current_entity.reset();
				ImGui::CloseCurrentPopup();
			}
		}
		ImGui::EndPopup();
	}

	bool open_themes{};
	bool b_help{};

	bool entity_popup{};
	std::string popup_label{};

	if (editor_flags.test(EditorFlags::create_new_room) || p_context->flags.test(EditorContextFlags::new_room)) { ImGui::OpenPopup("New Room"); }
	if (ImGui::BeginPopupModal("New Room", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		editor_flags.reset(EditorFlags::create_new_room);
		p_context->flags.reset(EditorContextFlags::new_room);
		if (create_new_room()) { set_new_room(); }
	}

	// Main Menu
	if (ImGui::BeginMainMenuBar()) {
		bool new_popup{};
		bool save_as_popup{};
		if (ImGui::BeginMenu("File")) {
			if (ImGui::MenuItem("New", NULL, &new_popup)) {}

			// Always center this p_services->window when appearing
			ImVec2 center(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);
			ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			ImGui::Separator();
#ifdef _WIN32
			if (ImGui::MenuItem("Open")) {
				char filename[MAX_PATH];
				OPENFILENAME ofn;
				ZeroMemory(&filename, sizeof(filename));
				ZeroMemory(&ofn, sizeof(ofn));
				ofn.lStructSize = sizeof(ofn);
				ofn.hwndOwner = NULL; // If you have a p_services->window to center over, put its HANDLE here
				ofn.lpstrFilter = "Json Files\0*.json\0Any File\0*.*\0Folders\0\0";
				ofn.lpstrFile = filename;
				ofn.nMaxFile = MAX_PATH;
				ofn.lpstrTitle = "Select a .json file to load.";
				ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST;

				if (GetOpenFileNameA(&ofn)) {
					auto open_path = std::filesystem::path{filename};
					p_services->finder.paths.region = open_path.parent_path().filename().string();
					p_services->finder.paths.room_name = open_path.filename().string();
					console.add_log(std::string{"region: " + p_services->finder.paths.region}.c_str());
					console.add_log(std::string{"filename: " + p_services->finder.paths.room_name}.c_str());
					load();
				} else {
					switch (CommDlgExtendedError()) {
					case CDERR_DIALOGFAILURE: console.add_log("CDERR_DIALOGFAILURE"); break;
					case CDERR_FINDRESFAILURE: console.add_log("CDERR_FINDRESFAILURE"); break;
					case CDERR_INITIALIZATION: console.add_log("CDERR_INITIALIZATION"); break;
					case CDERR_LOADRESFAILURE: console.add_log("CDERR_LOADRESFAILURE"); break;
					case CDERR_LOADSTRFAILURE: console.add_log("CDERR_LOADSTRFAILURE"); break;
					case CDERR_LOCKRESFAILURE: console.add_log("CDERR_LOCKRESFAILURE"); break;
					case CDERR_MEMALLOCFAILURE: console.add_log("CDERR_MEMALLOCFAILURE"); break;
					case CDERR_MEMLOCKFAILURE: console.add_log("CDERR_MEMLOCKFAILURE"); break;
					case CDERR_NOHINSTANCE: console.add_log("CDERR_NOHINSTANCE"); break;
					case CDERR_NOHOOK: console.add_log("CDERR_NOHOOK"); break;
					case CDERR_NOTEMPLATE: console.add_log("CDERR_NOTEMPLATE"); break;
					case CDERR_STRUCTSIZE: console.add_log("CDERR_STRUCTSIZE"); break;
					case FNERR_BUFFERTOOSMALL: console.add_log("FNERR_BUFFERTOOSMALL"); break;
					case FNERR_INVALIDFILENAME: console.add_log("FNERR_INVALIDFILENAME"); break;
					case FNERR_SUBCLASSFAILURE: console.add_log("FNERR_SUBCLASSFAILURE"); break;
					default: console.add_log("You cancelled.");
					}
				}
			}
#endif
			ImGui::Separator();
			if (ImGui::MenuItem("Save", "Ctrl+S")) { save() ? console.add_log("File saved successfully.") : console.add_log("Encountered an error saving file!"); }
			if (ImGui::MenuItem("Save As", nullptr, &save_as_popup)) {}
			if (ImGui::MenuItem("Close", nullptr)) { flags.set(GlobalFlags::shutdown); }
			ImGui::EndMenu();
		}
		if (new_popup) { ImGui::OpenPopup("New Room"); }
		if (ImGui::BeginPopupModal("New Room", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
			editor_flags.reset(EditorFlags::create_new_room);
			p_context->flags.reset(EditorContextFlags::new_room);
			if (create_new_room()) { set_new_room(); }
		}

		if (save_as_popup) { ImGui::OpenPopup("Save As"); }
		if (ImGui::BeginPopupModal("Save As", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
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
				p_services->finder.paths.region = regbuffer;
				p_services->finder.paths.room_name = std::string{roombuffer} + ".json";
				save();
				ImGui::CloseCurrentPopup();
			}

			ImGui::TextUnformatted(regbuffer);
			ImGui::TextUnformatted(roombuffer);

			ImGui::EndPopup();
		}
		if (ImGui::BeginMenu("Edit")) {
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
			if (ImGui::MenuItem("Themes", "", &open_themes)) {}

			ImGui::EndMenu();
		}
		bool flag{};
		if (ImGui::BeginMenu("Insert")) {
			if (ImGui::MenuItem("Ambient Prop", NULL, &entity_popup)) { popup_label = "Ambient Prop"; }
			if (ImGui::MenuItem("Portal", NULL, &entity_popup)) { popup_label = "Portal"; }
			if (ImGui::MenuItem("Inspectable", NULL, &entity_popup)) { popup_label = "Inspectable"; }
			if (ImGui::MenuItem("Platform", NULL, &entity_popup)) { popup_label = "Platform"; }
			if (ImGui::MenuItem("Enemy", NULL, &entity_popup)) { popup_label = "Enemy"; }
			if (ImGui::MenuItem("Chest", NULL, &entity_popup)) { popup_label = "Chest"; }
			if (ImGui::MenuItem("Destructible", NULL, &entity_popup)) { popup_label = "Destructible"; }
			if (ImGui::MenuItem("Bed", NULL, &entity_popup)) { popup_label = "Bed"; }
			if (ImGui::MenuItem("Switch Block", NULL, &entity_popup)) { popup_label = "Switch Block"; }
			if (ImGui::MenuItem("Switch Button", NULL, &entity_popup)) { popup_label = "Switch Button"; }
			if (ImGui::MenuItem("Turret", NULL, &entity_popup)) { popup_label = "Turret"; }
			if (ImGui::MenuItem("Timer Block", NULL, &entity_popup)) { popup_label = "Timer Block"; }
			if (ImGui::MenuItem("Light", NULL, &entity_popup)) { popup_label = "Light"; }
			if (ImGui::MenuItem("NPC", NULL, &entity_popup)) { popup_label = "NPC"; }
			if (ImGui::MenuItem("Animator", NULL, &entity_popup)) { popup_label = "Animator"; }
			if (ImGui::MenuItem("Vine", NULL, &entity_popup)) { popup_label = "Vine"; }
			if (ImGui::MenuItem("Water", NULL, &entity_popup)) { popup_label = "Water"; }
			if (ImGui::MenuItem("Train", NULL, &entity_popup)) { popup_label = "Train"; }
			if (ImGui::MenuItem("Teleporter", NULL, &entity_popup)) { popup_label = "Teleporter"; }
			if (ImGui::MenuItem("Cutscene Trigger", NULL, &entity_popup)) { popup_label = "Cutscene Trigger"; }
			if (ImGui::MenuItem("Save Point")) {
				current_tool = std::move(std::make_unique<EntityEditor>(EntityMode::placer));
				current_tool->current_entity = std::make_unique<fornani::SavePoint>(*p_services, map.room_id);
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Player Placer")) {
				current_tool = std::move(std::make_unique<EntityEditor>(EntityMode::placer));
				current_tool->ent_type = EntityType::player_placer;
			}
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Tools")) {
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
			if (ImGui::MenuItem("Export Layer to .png")) { export_layer_texture(); }
			ImGui::Separator();
			if (ImGui::MenuItem("Demo fullscreen", "", &m_demo.fullscreen)) {}
			if (ImGui::MenuItem("Save and Launch Demo", "Ctrl+L")) {
				save();
				m_demo.trigger.start();
			}
			if (ImGui::MenuItem("Launch Demo without Saving")) { m_demo.trigger.start(); }
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("View")) {
			if (ImGui::MenuItem("Center Canvas", "R")) { center_map(); }
			ImGui::Separator();
			ImGui::MenuItem("Show Sidebar", "", &m_options.sidebar);
			ImGui::MenuItem("Show Console", "", &m_options.console);
			ImGui::MenuItem("Show Palette", "", &m_options.palette);
			ImGui::Separator();
			ImGui::Checkbox("Debug Overlay", &show_overlay);
			ImGui::Checkbox("Show Entities", &map.flags.show_entities);
			ImGui::Checkbox("Show Background", &map.flags.show_background);
			ImGui::Checkbox("Show Grid (Tab)", &map.flags.show_grid);
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu("Help")) {
			b_help = true;
			ImGui::EndMenu();
		}
		if (ImGui::Button("Metagrid")) { p_target_state = EditorStateType::metagrid; }
		// TODO: fix up dialogue later
		// if (ImGui::Button("Dialogue")) { p_target_state = EditorStateType::dialogue_editor; }

		ImGui::EndMainMenuBar();
	}

	ImVec2 center = ImGui::GetMainViewport()->GetCenter();
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (b_help) {
		ImGui::OpenPopup("Help");
		b_help = false;
	}
	if (ImGui::BeginPopupModal("Help", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		ImGui::SeparatorText("Editor Controls");
		ImGui::Text("Toggle Grid: Tab");
		ImGui::Text("Show Current Layer Only: Shift");
		ImGui::Text("Reset Zoom and Pan: R");
		ImGui::Text("Layer Behind: Shift+Up");
		ImGui::Text("Layer in Front: Shift+Down");
		ImGui::SeparatorText("Tool Controls");
		ImGui::Text("Brush Size -: A");
		ImGui::Text("Brush Size +: D");
		ImGui::Text("Brush: B");
		ImGui::Text("Eyedropper: Alt");
		ImGui::Text("Maruqee: M");
		ImGui::Text("Fill: F");
		ImGui::Text("Hand: H");
		ImGui::Text("Eraser: E");
		ImGui::Text("Entity Editor: N");
		ImGui::SeparatorText("Playtest Controls");
		ImGui::Text("Launch: Ctrl+L");
		ImGui::Text("Launch at Mouse Position: Shift");
		ImGui::Text("Fullscreen: Alt");
		ImGui::NewLine();
		if (ImGui::Button("Close")) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}

	if (open_themes) {
		ImGui::OpenPopup("Level Themes");
		open_themes = false;
	}
	ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
	if (ImGui::BeginPopupModal("Level Themes", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		static int music_selected{};
		static int ambience_selected{};
		static int atmosphere_selected{};
		static std::string music_str{};
		static std::string ambience_str{};
		static std::vector<std::string> atmosphere_list{};

		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
		ImGui::BeginChild("ChildM", ImVec2(400, 600), true, ImGuiWindowFlags_None);
		if (ImGui::BeginTabBar("##themebar")) {
			if (ImGui::BeginTabItem("Style")) {
				for (auto const& choice : m_services->data.biomes["biomes"].as_array()) {
					if (ImGui::MenuItem(std::string{choice.as_string()}.c_str())) { map.biome = m_services->data.construct_biome(choice.as_string()); }
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Background")) {
				for (auto const& [key, entry] : m_services->data.background.as_object()) {
					if (ImGui::MenuItem(key.c_str())) { map.background = std::make_unique<fornani::graphics::Background>(*m_services, key); }
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Music")) {
				auto i = 0;
				for (auto [i, entry] : std::views::enumerate(m_services->data.audio_library["music"].as_array())) {
					ImGui::PushID(i);
					if (ImGui::ArrowButton(std::to_string(i).c_str(), ImGuiDir::ImGuiDir_Right)) {
						m_services->music_player.load(m_services->finder, entry.as_string());
						m_services->music_player.play_looped();
					}
					ImGui::SameLine();
					if (ImGui::Selectable(entry.as_string().c_str(), i == music_selected, ImGuiSelectableFlags_DontClosePopups)) {
						music_str = entry.as_string();
						music_selected = i;
					}
					ImGui::PopID();
					++i;
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Ambience")) {
				m_services->music_player.pause();
				for (auto [i, entry] : std::views::enumerate(m_services->data.audio_library["ambience"].as_array())) {
					if (ImGui::Selectable(entry.as_string().c_str(), i == ambience_selected, ImGuiSelectableFlags_DontClosePopups)) {
						ambience_str = entry.as_string();
						ambience_selected = i;
					}
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Atmosphere")) {
				m_services->music_player.pause();
				for (auto [i, entry] : std::views::enumerate(m_services->data.biomes["atmosphere"].as_array())) {
					if (ImGui::Selectable(entry.as_string().c_str(), map.has_atmosphere(entry.as_string()), ImGuiSelectableFlags_DontClosePopups)) {
						map.has_atmosphere(entry.as_string()) ? map.remove_atmosphere(entry.as_string()) : map.add_atmosphere(entry.as_string());
					} else {
					}
				}
				ImGui::EndTabItem();
			}
			if (ImGui::BeginTabItem("Weather")) {
				m_services->music_player.pause();
				map.report_weather();
				ImGui::EndTabItem();
			}
			ImGui::EndTabBar();
		}
		ImGui::PopStyleVar();
		ImGui::EndChild();

		ImGui::Separator();

		if (ImGui::Button("OK")) {
			map.set_music(music_str);
			map.set_ambience(ambience_str);
			m_services->music_player.pause();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel")) {
			m_services->music_player.pause();
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	std::string label = popup_label + " Specifications";
	if (entity_popup) { ImGui::OpenPopup(label.c_str()); }

	popup.launch(*p_services, p_services->finder, console, label.c_str(), current_tool, map.room_id);

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;

	ImVec2 window_pos;
	ImVec2 prev_window_pos{};
	ImVec2 prev_window_size{};
	window_pos.x = work_pos.x + palette.dimensions.x * palette.f_cell_size() + 2 * PAD;
	window_pos.y = PAD;
	window_flags |= ImGuiWindowFlags_NoMove;

	ImGui::SetNextWindowBgAlpha(m_menu_alpha);
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
					ImGui::Text("paths/resources: %s", p_services->finder.paths.resources.string().c_str());
					ImGui::Text("paths/editor...: %s", p_services->finder.paths.editor.string().c_str());
					ImGui::Text("paths/levels...: %s", p_services->finder.paths.levels.string().c_str());
					ImGui::Text("region: %s", p_services->finder.paths.region.c_str());
					ImGui::Text("room..: %s", p_services->finder.paths.room_name.c_str());
					ImGui::Separator();
					ImGui::Text("Any Widget Hovered: %s", is_any_widget_hovered() ? "Yes" : "No");
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
					ImGui::Text("Current Style: %s", map.biome.get_label());
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Tool")) {
					ImGui::Text("Left Mouse Clicked....: %s", p_mouse_cooldowns.left_click.running() ? "Yes" : "");
					ImGui::Text("Left Mouse Held.......: %s", p_left_mouse.held ? "Yes" : "");
					ImGui::Text("Left Mouse Released...: %s", p_mouse_cooldowns.left_release.running() ? "Yes" : "");
					ImGui::Text("Right Mouse Clicked...: %s", p_mouse_cooldowns.right_click.running() ? "Yes" : "");
					ImGui::Text("Right Mouse Held......: %s", p_right_mouse.held ? "Yes" : "");
					ImGui::Text("Right Mouse Released..: %s", p_mouse_cooldowns.right_release.running() ? "Yes" : "");
					ImGui::Text("Control Held...: %s", p_control.held ? "Yes" : "");
					ImGui::Text("Alt Held.......: %s", p_alt.held ? "Yes" : "");
					ImGui::Text("Shift Held.....: %s", p_shift.held ? "Yes" : "");
					ImGui::Separator();
					if (current_tool->current_entity) { ImGui::Text("entity moved: %b", current_tool->current_entity.value()->moved); }
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
					ImGui::Text("Map available? %s", map.is_available() ? "Yes" : "No");
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
		ImGui::SetNextWindowBgAlpha(m_menu_alpha); // Transparent background
		work_pos = viewport->WorkPos;			   // Use work area to avoid menu-bar/task-bar, if any!
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
			auto tools = sf::Sprite{p_services->assets.get_texture("editor_tools")};
			tools.setScale(sf::Vector2f{constants::ui_tool_scale_v, constants::ui_tool_scale_v});
			for (int i = 0; i < static_cast<int>(ToolType::eyedropper); i++) {
				ImGui::PushID(i);
				tools.setTextureRect(sf::IntRect{{i * constants::tool_size_v, 0}, {constants::tool_size_v, constants::tool_size_v}});
				ImGui::ImageButton(std::to_string(i).c_str(), tools, ImVec2{constants::tool_size_v * constants::ui_tool_scale_v, constants::tool_size_v * constants::ui_tool_scale_v}, sf::Color::Transparent, sf::Color::White);
				// TODO: get labels appropriately, if I decide I want them
				/*if (ImGui::IsItemHovered()) {
					ImGui::BeginTooltip();
					ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
					ImGui::Text(current_tool->get_label().c_str());
					ImGui::PopTextWrapPos();
					ImGui::EndTooltip();
				}*/
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
			ImGui::Text("Current Tile:");
			auto tileset = sf::Sprite{tileset_textures.at(map.get_i_style())};
			tileset.setTextureRect(sf::IntRect({palette.get_tile_coord(selected_block), fornani::constants::i_resolution_vec}));
			tileset.setScale(fornani::constants::f_scale_vec);
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
			ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
			ImGui::BeginChild("ChildS", ImVec2(320, 172), true, window_flags);
			ImGui::BeginMenuBar();
			if (ImGui::BeginMenu("Actions")) {
				if (ImGui::MenuItem("Insert Layer in Front")) { map.get_layers().add_layer(active_layer, 1); }
				if (ImGui::MenuItem("Insert Layer Behind")) { map.get_layers().add_layer(active_layer, 0); }
				if (ImGui::MenuItem("Delete Current Layer")) { delete_current_layer(); }
				reset_layers();
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Properties")) {
				ImGui::Text("Middleground: ");
				ImGui::SameLine();
				if (ImGui::InputInt("##smg", &m_middleground)) {
					m_middleground = std::clamp(m_middleground, 0, static_cast<int>(map.get_layers().layers.size()) - 1);
					map.get_layers().set_middleground(m_middleground);
				}
				auto ho{map.get_layers().m_flags.has_obscuring_layer};
				if (ImGui::MenuItem("Include Obscuring Layer", "", &map.get_layers().m_flags.has_obscuring_layer)) {
					if (map.get_layers().m_flags.has_reverse_obscuring_layer && !map.get_layers().m_flags.has_obscuring_layer) { map.get_layers().m_flags.has_reverse_obscuring_layer = false; }
					if (map.get_layers().layers.size() - m_middleground < 2 && !ho) { map.get_layers().add_layer(m_middleground, 1); }
				}
				auto hro{map.get_layers().m_flags.has_reverse_obscuring_layer};
				if (ImGui::MenuItem("Include Reverse Obscuring Layer", "", &map.get_layers().m_flags.has_reverse_obscuring_layer)) {
					if (map.get_layers().m_flags.has_reverse_obscuring_layer && !map.get_layers().m_flags.has_obscuring_layer) { map.get_layers().m_flags.has_obscuring_layer = true; }
					if (!hro) {
						if (map.get_layers().layers.size() - m_middleground < 3) {
							map.get_layers().add_layer(m_middleground, 1);
							map.get_layers().add_layer(m_middleground, 1);
						} else if (map.get_layers().layers.size() - m_middleground < 2) {
							map.get_layers().add_layer(m_middleground, 1);
						}
					}
				}
				ImGui::Separator();
				static bool mp_randomness{map.test_property(fornani::world::MapProperties::environmental_randomness)};
				static bool mp_shift{map.test_property(fornani::world::MapProperties::day_night_shift)};
				static bool mp_lighting{map.test_property(fornani::world::MapProperties::lighting)};
				static bool mp_int{map.test_property(fornani::world::MapProperties::interior)};
				static bool mp_tox{map.test_property(fornani::world::MapProperties::toxic)};
				static int darken = static_cast<float>(darken);
				if (b_reloaded) {
					mp_randomness = map.test_property(fornani::world::MapProperties::environmental_randomness);
					mp_shift = map.test_property(fornani::world::MapProperties::day_night_shift);
					mp_lighting = map.test_property(fornani::world::MapProperties::lighting);
					mp_int = map.test_property(fornani::world::MapProperties::interior);
					mp_tox = map.test_property(fornani::world::MapProperties::toxic);
					darken = static_cast<int>(map.darken_factor);
				}
				if (ImGui::MenuItem("Environmental Randomness", "", &mp_randomness)) {}
				if (ImGui::MenuItem("Day Night Shift", "", &mp_shift)) {}
				if (ImGui::MenuItem("Toxic", "", &mp_tox)) {}
				if (ImGui::MenuItem("Interior", "", &mp_int)) {}
				if (ImGui::MenuItem("Lighting", "", &mp_lighting)) {}
				ImGui::Text("Shadow Level: ");
				ImGui::SameLine();
				if (ImGui::InputInt("##slv", &darken)) { map.darken_factor = static_cast<float>(darken); }
				mp_randomness ? map.set_property(fornani::world::MapProperties::environmental_randomness) : map.reset_property(fornani::world::MapProperties::environmental_randomness);
				mp_shift ? map.set_property(fornani::world::MapProperties::day_night_shift) : map.reset_property(fornani::world::MapProperties::day_night_shift);
				mp_lighting ? map.set_property(fornani::world::MapProperties::lighting) : map.reset_property(fornani::world::MapProperties::lighting);
				mp_int ? map.set_property(fornani::world::MapProperties::interior) : map.reset_property(fornani::world::MapProperties::interior);
				mp_tox ? map.set_property(fornani::world::MapProperties::toxic) : map.reset_property(fornani::world::MapProperties::toxic);
				reset_layers();
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

			// Palette Modes
			ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);
			ImGui::BeginChild("ChildM", ImVec2(320, 52), true, ImGuiWindowFlags_None);
			if (ImGui::Selectable("Tile", current_tool->is_mode(BrushMode::tile))) {
				m_mode = BrushMode::tile;
				m_options.palette = true;
			}
			if (ImGui::Selectable("Hazard", current_tool->is_mode(BrushMode::hazard))) { m_mode = BrushMode::hazard; }
			ImGui::EndChild();
			ImGui::PopStyleVar();
			ImGui::SeparatorText("Settings");
			if (ImGui::BeginTabBar("##globset")) {
				if (ImGui::BeginTabItem("Canvas")) {
					if (ImGui::BeginTabBar("##gensettings")) {
						if (ImGui::BeginTabItem("Layer Properties")) {
							if (ImGui::Checkbox("Ignore Lighting", &map.get_active_layer().ignore_lighting)) {};
							ImGui::SliderFloat("Parallax Factor", &map.get_active_layer().parallax, 0.f, 1.f);
							ImGui::EndTabItem();
						}
						ImGui::EndTabBar();
					}

					prev_window_size = ImGui::GetWindowSize();
					prev_window_pos = ImGui::GetWindowPos();

					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Editor")) {
					window_flags = ImGuiWindowFlags_None;
					if (ImGui::BeginTabBar("##prf")) {
						if (ImGui::BeginTabItem("Visual")) {
							ImGui::Checkbox("Show Entities", &map.flags.show_entities);
							ImGui::Checkbox("Show Background", &map.flags.show_background);
							ImGui::Checkbox("Show Grid (Tab)", &map.flags.show_grid);
							ImGui::Checkbox("Show All Layers (Shift)", &map.flags.show_all_layers);
							ImGui::Checkbox("Show Obscuring Layer", &map.flags.show_obscured_layer);
							ImGui::Checkbox("Show Reverse Obscuring Layer", &map.flags.show_reverse_obscured_layer);
							ImGui::Checkbox("Show Indicated Layers", &map.flags.show_indicated_layers);
							ImGui::EndTabItem();
						}
						if (ImGui::BeginTabItem("Config")) {
							ImGui::SliderFloat("Widget Alpha", &m_menu_alpha, 0.f, 1.f);
							ImGui::Checkbox("Debug Overlay", &show_overlay);
							auto r = static_cast<float>(p_colors.backdrop.r) / 255.f;
							auto g = static_cast<float>(p_colors.backdrop.g) / 255.f;
							auto b = static_cast<float>(p_colors.backdrop.b) / 255.f;
							static float wallpaper[3] = {r, g, b};
							ImGui::ColorEdit3("Wallpaper", wallpaper);
							auto wr = static_cast<std::uint8_t>(wallpaper[0] * 255.f);
							auto wg = static_cast<std::uint8_t>(wallpaper[1] * 255.f);
							auto wb = static_cast<std::uint8_t>(wallpaper[2] * 255.f);
							p_colors.backdrop = sf::Color{wr, wg, wb};
							ImGui::EndTabItem();
						}
						ImGui::EndTabBar();
					}

					prev_window_size = ImGui::GetWindowSize();
					prev_window_pos = ImGui::GetWindowPos();
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}
			ImGui::SeparatorText("Playtest");
			auto& e = m_services->editor_settings.save_file;
			ImGui::RadioButton("file 1", &e, 0);
			ImGui::RadioButton("file 2", &e, 1);
			ImGui::RadioButton("file 3", &e, 2);
			ImGui::SeparatorText("Info");
			ImGui::Text("Current Room ID: %u", map.room_id);
			ImGui::SeparatorText("Development");
			auto& status = map.get_status();
			ImGui::RadioButton("unfinished", &status, 0);
			ImGui::RadioButton("prototype", &status, 1);
			ImGui::RadioButton("production", &status, 2);

			ImGui::End();
		}
		if (m_options.console) { console.write_console(prev_window_size, prev_window_pos, m_menu_alpha); }
	}

	if (current_tool->type == ToolType::entity_editor && !is_any_widget_hovered() && current_tool->entity_mode != EntityMode::editor && !current_tool->entity_menu) {
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::Text(current_tool->get_tooltip().c_str());
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}

	b_reloaded = false;
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
				auto x_coord = (cell.value % 16) * 32;
				auto y_coord = std::floor(cell.value / 16) * 32;
				auto tile_sprite = sf::Sprite{tileset_textures.at(map.biome.get_id())};
				tile_sprite.setTextureRect(sf::IntRect({static_cast<int>(x_coord), static_cast<int>(y_coord)}, {32, 32}));
				tile_sprite.setPosition(cell.scaled_position());
				screencap.draw(tile_sprite);
			}
		}
	}
	std::time_t time = std::time(nullptr);
	std::string time_str(21, '\0');
	std::strftime(time_str.data(), time_str.size(), "%FT%TZ", std::gmtime(&time));
	time_str.resize(std::strlen(time_str.c_str()));

	std::erase_if(time_str, [](auto const& c) { return c == ':' || isspace(c); });
	std::string filename = "screenshot_" + time_str + ".png";
	if (screencap.getTexture().copyToImage().saveToFile(filename)) {
		std::string log = "Screenshot saved to " + filename + "\n";
		console.add_log(log.data());
	}
}

void Editor::center_map() {
	map.center(p_services->window->f_center_screen());
	map.set_scale(1.f);
}

void Editor::launch_demo(char** argv, int room_id, std::filesystem::path path, sf::Vector2f player_position) {
	m_demo.custom_position = false;
	current_tool->current_entity = {};
	ImGui::SFML::Shutdown();
	fornani::Application demo{argv};
	console.add_log("> Launching Demo");
	console.add_log(std::string{"Room ID: " + std::to_string(room_id) + "; Room Name: " + p_services->finder.paths.room_name}.c_str());
	demo.init(argv, {true, m_demo.fullscreen});
	demo.set_file(m_services->editor_settings.save_file);
	demo.launch(argv, true, room_id, p_services->finder.paths.room_name, player_position);
}

void Editor::reset_layers() {
	map.get_layers().set_labels();
	for (std::size_t i = 0; i < map.get_layers().layers.size(); ++i) {
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
	if (map.get_layers().m_flags.has_reverse_obscuring_layer && layers.at(active_layer).render_order == layers.size() - 2) { map.get_layers().m_flags.has_reverse_obscuring_layer = false; }
	if (map.get_layers().m_flags.has_obscuring_layer && layers.at(active_layer).render_order == layers.size() - 1) {
		map.get_layers().m_flags.has_obscuring_layer = false;
		map.get_layers().m_flags.has_reverse_obscuring_layer = false;
	}
	map.save_state(*current_tool, true);
	map.get_layers().delete_layer_at(active_layer);
	reset_layers();
	if (layers.size() <= 1) {
		active_layer = 0;
		return;
	}
	active_layer = std::clamp(active_layer > 0 ? active_layer - 1 : std::size_t{0}, std::size_t{0}, map.get_layers().layers.size() - 1);
}

void Editor::set_new_room() {
	static std::string style_current = std::string{map.biome.get_label()};
	static std::string bg_current = map.background->get_label();

	map = Canvas(*p_services, {static_cast<std::uint32_t>(width * chunk_size_v), static_cast<std::uint32_t>(height * chunk_size_v)}, SelectionType::canvas, m_services->data.construct_biome(style_current), bg_current);
	map.metagrid_coordinates = p_context->metagrid_position;
	p_services->finder.paths.region = regbuffer;
	p_services->finder.paths.room_name = std::string{roombuffer} + ".json";
	map.room_id = p_new_id;
	save();
	load();
	reset_layers();
	map.center(p_services->window->f_center_screen());
	dj::Json this_room{};
	this_room["room_id"] = p_new_id;
	this_room["region"] = p_services->finder.paths.region;
	this_room["label"] = p_services->finder.paths.room_name;
	this_room["minimap"] = true;
	p_services->data.map_table["rooms"].push_back(this_room);
	console.add_log(std::string{"In folder " + p_services->finder.paths.region}.c_str());
	console.add_log(std::string{"Created new room with id " + std::to_string(p_new_id) + " and name " + p_services->finder.paths.room_name}.c_str());

	ImGui::CloseCurrentPopup();
}

} // namespace pi
