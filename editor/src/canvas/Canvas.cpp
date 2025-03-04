
#include "editor/canvas/Canvas.hpp"
#include "editor/util/Lookup.hpp"
#include "editor/tool/Tool.hpp"
#include "fornani/setup/ResourceFinder.hpp"
#include <cassert>

namespace pi {

Canvas::Canvas(fornani::data::ResourceFinder& finder, SelectionType type, StyleType style, Backdrop backdrop, int num_layers) : Canvas(finder, {}, type, style, backdrop, num_layers) {}

Canvas::Canvas(fornani::data::ResourceFinder& finder, sf::Vector2<uint32_t> dim, SelectionType type, StyleType style, Backdrop backdrop, int num_layers) : type(type), styles{.tile{style}}, background{std::make_unique<Background>(finder, backdrop)} {
	type == SelectionType::canvas ? properties.set(CanvasProperties::editable) : properties.reset(CanvasProperties::editable);
    dimensions = dim;
	real_dimensions = {static_cast<float>(dim.x) * f_cell_size(), static_cast<float>(dim.y) * f_cell_size()};
    clear();
    map_states.push_back(Map());

	for (auto i{0}; i < num_layers; ++i) { map_states.back().layers.push_back(Layer(i, i == default_middleground_v, dim)); }
	map_states.back().set_middleground(default_middleground_v);
	map_states.back().set_labels();

	box.setOutlineColor(sf::Color{200, 200, 200, 20});
	box.setOutlineThickness(-2);
	box.setSize({f_cell_size(), f_cell_size()});

	gridbox.setFillColor(sf::Color::Transparent);
	gridbox.setOutlineColor(sf::Color{255, 255, 255, 255});
	chunkbox.setFillColor(sf::Color::Transparent);
	chunkbox.setOutlineColor(sf::Color{80, 255, 160, 255});

	border.setFillColor(sf::Color::Transparent);
	border.setOutlineThickness(4.f);
}

void Canvas::update(Tool& tool) {
	real_dimensions = {static_cast<float>(dimensions.x) * f_native_cell_size(), static_cast<float>(dimensions.y) * f_native_cell_size()};
	if (editable()) {
		tool.in_bounds(dimensions) ? state.set(CanvasState::hovered) : state.reset(CanvasState::hovered);
	} else {
		within_bounds(tool.get_window_position()) ? state.set(CanvasState::hovered) : state.reset(CanvasState::hovered);
	}
	background->update();

	// update grid
	for (auto& layer : get_layers().layers) { layer.set_position({}, f_native_cell_size()); }
}

void Canvas::render(sf::RenderWindow& win, sf::Sprite& tileset) {
	if (flags.show_background) { background->render(*this, win, position); }
	border.setPosition(get_position());
	hovered() ? border.setOutlineColor({240, 230, 255, 80}) : border.setOutlineColor({240, 230, 255, 40});
	border.setSize(get_real_dimensions());
	win.draw(border);
	if (!states_empty()) {
		for (auto& layer : get_layers().layers) {
			box.setFillColor(sf::Color{40, 240, 80, 20});
			for (auto& cell : layer.grid.cells) {
				cell.set_scale(scale);
				if (cell.value == 0) { continue; }
				if (layer.render_order == active_layer || flags.show_all_layers) {
					auto squared = scale == 1.f ? 1.f : 1.01f;
					tileset.setTextureRect(sf::IntRect{get_tile_coord(cell.value), {32, 32}});
					tileset.setScale({scale * squared, scale * squared});
					tileset.setOrigin(get_origin());
					tileset.setPosition(cell.scaled_position() + position);
					if (layer.render_order == get_layers().layers.size() - 1) {
						if (flags.show_obscured_layer) { win.draw(tileset); }
					} else if (layer.render_order == get_layers().layers.size() - 2) {
						if (flags.show_reverse_obscured_layer) { win.draw(tileset); }
					} else {
						win.draw(tileset);
					}
				} else if (flags.show_indicated_layers) {
					box.setScale({scale, scale});
					box.setOrigin(get_origin());
					box.setPosition(cell.scaled_position() + position);
					win.draw(box);
				}
			}
		}
	}
	if (flags.show_entities) { entities.render(*this, win, get_position()); }
	if (flags.show_grid && !states_empty()) {
		auto grid_sprite = sf::Sprite(grid_texture.getTexture());
		grid_sprite.setPosition(position);
		grid_sprite.setOrigin(get_origin());
		grid_sprite.setScale({scale, scale});
		grid_sprite.setColor(sf::Color{255, 255, 255, 20});
		win.draw(grid_sprite);
	}
}

bool Canvas::load(fornani::data::ResourceFinder& finder, std::string const& region, std::string const& room_name, bool local) {

	auto success{true};
	map_states.clear();
	redo_states.clear();

	// init map_states
	map_states.push_back(Map());
	clear();

	auto const& source = local ? finder.paths.editor : finder.paths.levels;

	std::string metapath = (source / std::filesystem::path{region} / std::filesystem::path{room_name}).string();

	data.meta = dj::Json::from_file((metapath).c_str());
	if (data.meta.is_null()) {
		finder.paths.region = "config";
		finder.paths.room_name = "new_file.json";
		metapath = (source / finder.paths.region / finder.paths.room_name).string();
		data.meta = dj::Json::from_file((metapath).c_str());
		success = false;
	}
	assert(!data.meta.is_null());

	if (!local) { entities = EntitySet{finder, data.meta["entities"], room_name}; }

	auto const& meta = data.meta["meta"];
	room_id = meta["room_id"].as<int>();
	minimap = static_cast<bool>(meta["minimap"].as_bool());
	metagrid_coordinates.x = meta["metagrid"][0].as<int>();
	metagrid_coordinates.y = meta["metagrid"][1].as<int>();
	dimensions.x = meta["dimensions"][0].as<int>();
	dimensions.y = meta["dimensions"][1].as<int>();
	real_dimensions = {static_cast<float>(dimensions.x) * constants.cell_size, static_cast<float>(dimensions.y) * constants.cell_size};
	auto style_value = meta["style"].as<int>();
	styles.tile = Style(static_cast<StyleType>(style_value));
	m_theme.music = meta["music"].as_string();
	m_theme.ambience = meta["ambience"].as_string();
	for (auto& in : meta["atmosphere"].array_view()) { m_theme.atmosphere.push_back(in.as<int>()); };
	if (meta["camera_effects"]) {
		m_camera_effects.shake_properties.frequency = meta["camera_effects"]["shake"]["frequency"].as<int>();
		m_camera_effects.shake_properties.energy = meta["camera_effects"]["shake"]["energy"].as<float>();
		m_camera_effects.shake_properties.start_time = meta["camera_effects"]["shake"]["start_time"].as<float>();
		m_camera_effects.shake_properties.dampen_factor = meta["camera_effects"]["shake"]["dampen_factor"].as<int>();
		m_camera_effects.frequency_in_seconds = meta["camera_effects"]["shake"]["frequency_in_seconds"].as<int>();
	}
	if (meta["cutscene_on_entry"]) {
		cutscene.flag = static_cast<bool>(meta["cutscene_on_entry"]["flag"].as_bool());
		cutscene.type = meta["cutscene_on_entry"]["type"].as<int>();
		cutscene.id = meta["cutscene_on_entry"]["id"].as<int>();
		cutscene.source = meta["cutscene_on_entry"]["source"].as<int>();
	}
	if (meta["background"]) { background = std::make_unique<Background>(finder, static_cast<Backdrop>(meta["background"].as<int>())); }

    // tiles
	auto counter{0};
	for (auto& layer : data.meta["tile"]["layers"].array_view()) {
		map_states.back().layers.push_back(Layer(counter, counter == map_states.back().get_middleground(), dimensions));
        int cell_counter{};
        for (auto& cell : layer.array_view()) {
            map_states.back().layers.back().grid.cells.at(cell_counter).value = cell.as<int>();
            ++cell_counter;
        }
		++counter;
	}
	map_states.back().set_middleground(data.meta["tile"]["middleground"].as<int>());
	map_states.back().m_flags.has_obscuring_layer = static_cast<bool>(data.meta["tile"]["flags"]["obscuring"].as_bool());
	map_states.back().m_flags.has_reverse_obscuring_layer = static_cast<bool>(data.meta["tile"]["flags"]["reverse_obscuring"].as_bool());
	entities.variables.player_start = map_states.back().layers.at(middleground()).grid.first_available_ground();
	map_states.back().set_labels();
	set_grid_texture();
	return success;
}

bool Canvas::save(fornani::data::ResourceFinder& finder, std::string const& region, std::string const& room_name) {

	std::filesystem::create_directory(finder.paths.levels / std::filesystem::path{region});

	// clean jsons
	data = {};

	// empty json array
	constexpr auto empty_array = R"([])";
	auto const wipe = dj::Json::parse(empty_array);

	// data.meta
	data.meta["meta"]["room_id"] = room_id;
	data.meta["meta"]["minimap"] = dj::Boolean{minimap};
	data.meta["meta"]["metagrid"][0] = metagrid_coordinates.x;
	data.meta["meta"]["metagrid"][1] = metagrid_coordinates.y;
	data.meta["meta"]["dimensions"][0] = dimensions.x;
	data.meta["meta"]["dimensions"][1] = dimensions.y;
	data.meta["meta"]["style"] = static_cast<int>(styles.tile.get_type());
	data.meta["meta"]["background"] = static_cast<int>(background->type.get_type());
	data.meta["meta"]["music"] = m_theme.music;
	for (auto& entry : m_theme.atmosphere) { data.meta["meta"]["atmosphere"].push_back(entry); }
	data.meta["meta"]["ambience"] = m_theme.ambience;
	data.meta["meta"]["camera_effects"]["shake"]["frequency"] = m_camera_effects.shake_properties.frequency;
	data.meta["meta"]["camera_effects"]["shake"]["energy"] = m_camera_effects.shake_properties.energy;
	data.meta["meta"]["camera_effects"]["shake"]["start_time"] = m_camera_effects.shake_properties.start_time;
	data.meta["meta"]["camera_effects"]["shake"]["dampen_factor"] = m_camera_effects.shake_properties.dampen_factor;
	data.meta["meta"]["camera_effects"]["shake"]["frequency_in_seconds"] = m_camera_effects.frequency_in_seconds;
	data.meta["meta"]["cutscene_on_entry"]["flag"] = dj::Boolean{cutscene.flag};
	data.meta["meta"]["cutscene_on_entry"]["type"] = cutscene.type;
	data.meta["meta"]["cutscene_on_entry"]["id"] = cutscene.id;
	data.meta["meta"]["cutscene_on_entry"]["source"] = cutscene.source;

	data.meta["tile"]["layers"] = wipe;
	for (auto i{0}; i < last_layer(); ++i) { data.meta["tile"]["layers"].push_back(wipe); }
	// push layer data
	int current_layer{};
	data.meta["tile"]["flags"]["obscuring"] = dj::Boolean{map_states.back().m_flags.has_obscuring_layer};
	data.meta["tile"]["flags"]["reverse_obscuring"] = dj::Boolean{map_states.back().m_flags.has_reverse_obscuring_layer};
	for (auto& layer : map_states.back().layers) {
		if (map_states.back().get_middleground() == current_layer) { data.meta["tile"]["middleground"] = current_layer; }
		int current_cell{};
		for ([[maybe_unused]] auto& cell : layer.grid.cells) {
			data.meta["tile"]["layers"][current_layer].push_back(layer.grid.cells.at(current_cell).value);
			++current_cell;
		}
		++current_layer;
	}

	auto success{true};
	if (!entities.save(finder, data.meta["entities"], room_name)) { success = false; }
	if (!data.meta.to_file((finder.paths.levels / region / room_name).string().c_str())) { success = false; }
	return success;
}

void Canvas::clear() {
    if (!map_states.empty()) {
		for (auto& layer : map_states.back().layers) { layer.erase(); }
        map_states.back().layers.clear();
    }
	m_theme.atmosphere.clear();
}

void Canvas::save_state(Tool& tool, bool force) {
    auto const& type = tool.type;
    auto undoable_tool = type == ToolType::brush || type == ToolType::fill || type == ToolType::marquee || type == ToolType::erase;
	if ((undoable_tool && tool.clicked() && editable()) || force) {
		map_states.emplace_back(Map{map_states.back()});
		if (map_states.size() > max_undo_states_v) { map_states.pop_front(); }
        clear_redo_states();
    }
}

void Canvas::undo() {
    if (map_states.size() > 1) {
        redo_states.push_back(map_states.back());
        map_states.pop_back();
		dimensions = map_states.back().layers.back().dimensions;
	}
	set_grid_texture();
}

void Canvas::redo() {
    if (redo_states.size() > 0) {
        map_states.push_back(redo_states.back());
		redo_states.pop_back();
		dimensions = map_states.back().layers.back().dimensions;
	}
	set_grid_texture();
}

void Canvas::clear_redo_states() { redo_states.clear(); }

void Canvas::hover() { state.set(CanvasState::hovered); }

void Canvas::unhover() { state.reset(CanvasState::hovered); }

void Canvas::move(sf::Vector2<float> distance) { position += distance; }

void Canvas::set_position(sf::Vector2<float> to_position) { position = to_position; }

void Canvas::set_origin(sf::Vector2<float> to_origin) { origin = to_origin; }

void Canvas::set_offset_from_center(sf::Vector2<float> offset) { offset_from_center = offset; }

void Canvas::set_scale(float to_scale) { scale = to_scale; }

void Canvas::resize(sf::Vector2i adjustment) {
	// map dimensions can't be 0
	if (adjustment.x < 0 && chunk_dimensions().x == 1) { return; }
	if (adjustment.y < 0 && chunk_dimensions().y == 1) { return; }
	auto current = Map{map_states.back()};
	dimensions.x += adjustment.x * static_cast<int>(f_native_chunk_size());
	dimensions.y += adjustment.y * static_cast<int>(f_native_chunk_size());
	auto num_layers = map_states.back().layers.size();
	map_states.push_back(Map());
	for (auto i{0}; i < num_layers; ++i) {
		map_states.back().layers.push_back(Layer(i, i == current.get_middleground(), dimensions));
		map_states.back().layers.back().grid.match(current.layers.at(i).grid);
	}
	clear_redo_states();
	set_grid_texture();
}

void Canvas::center(sf::Vector2<float> point) { set_position(point - real_dimensions * 0.5f); }

void Canvas::constrain(sf::Vector2<float> bounds) {
	position.x = std::clamp(position.x, -get_real_dimensions().x, bounds.x);
	position.y = std::clamp(position.y, -get_real_dimensions().y, bounds.y);
}

void Canvas::zoom(float amount) { scale = std::clamp(scale + amount, min_scale, max_scale); }

void Canvas::set_backdrop_color(sf::Color color) { border.setFillColor(color); }

void Canvas::set_grid_texture() {
	if (get_layers().layers.empty()) { return; }
	if (!grid_texture.resize(sf::Vector2u{static_cast<uint32_t>(dimensions.x * f_native_cell_size()), static_cast<uint32_t>(dimensions.y * f_native_cell_size())})) {
		NANI_LOG_WARN(m_logger, "Failed to resize grid texture.");
		return;
	}
	grid_texture.clear(sf::Color::Transparent);
	for (auto i{0}; i < get_layers().layers.back().grid.dimensions.x; ++i) {
		for (auto j{0}; j < get_layers().layers.back().grid.dimensions.y; ++j) {
			gridbox.setPosition({static_cast<float>(i * f_native_cell_size()), static_cast<float>(j * f_native_cell_size())});
			if (i % i_native_chunk_size() == 0 && j % i_native_chunk_size() == 0 && editable()) {
				chunkbox.setPosition(gridbox.getPosition());
				chunkbox.setSize({f_cell_size() * f_native_chunk_size(), f_cell_size() * f_native_chunk_size()});
				chunkbox.setScale({1.f / scale, 1.f / scale});
				chunkbox.setOutlineThickness(std::min(-scale * 2.f, -4.f));
				grid_texture.draw(chunkbox);
			}
			gridbox.setSize({f_cell_size(), f_cell_size()});
			gridbox.setScale({1.f / scale, 1.f / scale});
			gridbox.setOutlineThickness(std::min(-scale, -1.f));
			grid_texture.draw(gridbox);
		}
	}
	grid_texture.display();
}

void Canvas::activate_middleground() { map_states.back().layers.at(middleground()).active = true; }

Map& Canvas::get_layers() { return map_states.back(); }

sf::Vector2<int> Canvas::get_tile_coord(int lookup) {
	sf::Vector2<int> ret{};
	ret.x = static_cast<int>(lookup % 16);
	ret.y = static_cast<int>(std::floor(lookup / 16));
	return ret * 32;
}

void Canvas::replace_tile(uint32_t from, uint32_t to, int layer_index) {
	if (layer_index >= map_states.back().layers.size()) { return; }
	for (auto& tile : map_states.back().layers.at(layer_index).grid.cells) {
		if (tile.value == from) { tile.value = to; }
	}
}

void Canvas::edit_tile_at(int i, int j, int new_val, int layer_index) {
    if(layer_index >= map_states.back().layers.size()) { return; }
    if((i + j * dimensions.x) >= map_states.back().layers.at(layer_index).grid.cells.size()) { return; };
    map_states.back().layers.at(layer_index).grid.cells.at(i + j * dimensions.x).value = new_val;
}

void Canvas::erase_at(int i, int j, int layer_index) { edit_tile_at(i, j, 0, layer_index); }

int Canvas::tile_val_at(int i, int j, int layer) { return map_states.back().layers.at(layer).grid.cells.at(i + j * dimensions.x).value; }

int Canvas::tile_val_at_scaled(int i, int j, int layer) {
	auto u = std::floor(i / 32);
	auto v = std::floor(j / 32);
	auto idx = static_cast<std::size_t>(u + v * dimensions.x);
	if (idx < 0 || idx >= map_states.back().layers.at(layer).grid.cells.size()) { return 0; }
	return map_states.back().layers.at(layer).grid.cells.at(idx).value;
}

sf::Vector2<float> Canvas::get_tile_position_at(int i, int j, int layer) {
	auto u = std::floor(i / 32);
	auto v = std::floor(j / 32);
	auto idx = static_cast<std::size_t>(u + v * dimensions.x);
	if (idx < 0 || idx >= map_states.back().layers.at(layer).grid.cells.size()) { return {}; }
	return map_states.back().layers.at(layer).grid.cells.at(idx).scaled_position();
}

Tile& Canvas::get_tile_at(int i, int j, int layer) {
	auto u = std::floor(i / 32);
	auto v = std::floor(j / 32);
	auto idx = static_cast<std::size_t>(u + v * dimensions.x);
	if (idx < 0 || idx >= map_states.back().layers.at(layer).grid.cells.size()) { return map_states.back().layers.at(layer).grid.cells.at(0); }
	return map_states.back().layers.at(layer).grid.cells.at(idx);
}

}
