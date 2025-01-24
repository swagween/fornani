
#include "editor/canvas/Canvas.hpp"
#include "editor/util/Lookup.hpp"
#include "editor/tool/Tool.hpp"
#include "fornani/setup/ResourceFinder.hpp"
#include <cassert>

namespace pi {

Canvas::Canvas(SelectionType type) : Canvas({}, type) {}

Canvas::Canvas(sf::Vector2<uint32_t> dim, SelectionType type) : camera{sf::Vector2<float>{static_cast<float>(dim.x), static_cast<float>(dim.y)}}, type(type) {
	type == SelectionType::canvas ? properties.set(CanvasProperties::editable) : properties.reset(CanvasProperties::editable);
    dimensions = dim;
	real_dimensions = {static_cast<float>(dim.x) * f_cell_size(), static_cast<float>(dim.y) * f_cell_size()};
    clear();
    map_states.push_back(Map());
	for (uint32_t i = 0; i < NUM_LAYERS; ++i) { map_states.back().layers.push_back(Layer(i, (i == MIDDLEGROUND), dim)); }

	box.setOutlineColor(sf::Color{200, 200, 200, 20});
	box.setOutlineThickness(-2);
	box.setSize({f_cell_size(), f_cell_size()});

	gridbox.setFillColor(sf::Color::Transparent);
	gridbox.setOutlineColor(sf::Color{240, 230, 255, 20});
	gridbox.setOutlineThickness(-1);
	gridbox.setSize({f_cell_size(), f_cell_size()});

	chunkbox.setFillColor(sf::Color::Transparent);
	chunkbox.setOutlineColor(sf::Color{80, 230, 255, 16});
	chunkbox.setOutlineThickness(-2);
	chunkbox.setSize({f_cell_size() * f_native_chunk_size(), f_cell_size() * f_native_chunk_size()});

	border.setFillColor(sf::Color::Transparent);
	border.setOutlineThickness(4);
}

void Canvas::update(Tool& tool, bool transformed) {
	real_dimensions = {static_cast<float>(dimensions.x) * f_native_cell_size(), static_cast<float>(dimensions.y) * f_native_cell_size()};
	if (transformed) {
		within_bounds(tool.f_position()) ? state.set(CanvasState::hovered) : state.reset(CanvasState::hovered);
	} else {
		within_bounds(tool.get_window_position()) ? state.set(CanvasState::hovered) : state.reset(CanvasState::hovered);
	}
	camera.update();
	background->update();

	// update grid
	for (auto& layer : get_layers().layers) { layer.set_position({}, f_native_cell_size()); }
}

void Canvas::render(sf::RenderWindow& win, sf::Sprite& tileset) {
	if (flags.show_background) { background->render(*this, win, camera.position); }
	border.setPosition(get_position());
	hovered() ? border.setOutlineColor({240, 230, 255, 80}) : border.setOutlineColor({240, 230, 255, 40});
	border.setSize(get_real_dimensions());
	win.draw(border);
	if (!states_empty()) {
		for (auto& layer : get_layers().layers) {
			box.setFillColor(sf::Color{40, 240, 80, 40});
			for (auto& cell : layer.grid.cells) {
				cell.set_scale(scale);
				if (cell.value == 0) { continue; }
				if (layer.render_order == active_layer || flags.show_all_layers) {
					auto squared = scale == 1.f ? 1.f : 1.01f;
					tileset.setTextureRect(sf::IntRect{get_tile_coord(cell.value), {32, 32}});
					tileset.setScale({scale * squared, scale * squared});
					tileset.setOrigin(get_origin());
					tileset.setPosition(cell.scaled_position() + camera.position);
					if (layer.render_order != 7 || flags.show_obscured_layer) { win.draw(tileset); }
				} else if (flags.show_indicated_layers) {
					box.setScale({scale, scale});
					box.setOrigin(get_origin());
					box.setPosition(cell.scaled_position() + camera.position);
					win.draw(box);
				}
			}
		}
	}
	if (flags.show_entities) { entities.render(*this, win, get_position()); }
	if (flags.show_grid && !states_empty()) {
		if (get_layers().layers.empty()) { return; }
		for (auto& cell : get_layers().layers.back().grid.cells) {
			if (cell.scaled_position().x + camera.bounding_box.position.x < 0) { continue; }
			if (cell.scaled_position().x + camera.bounding_box.position.x > win.getSize().x) { continue; }
			if (cell.scaled_position().y + camera.bounding_box.position.y < 0) { continue; }
			if (cell.scaled_position().y + camera.bounding_box.position.y > win.getSize().y) { continue; }
			gridbox.setSize({f_cell_size(), f_cell_size()});
			gridbox.setOrigin(get_origin());
			gridbox.setPosition(cell.scaled_position() + camera.position);
			if (cell.grid_position().x % i_native_chunk_size() == 0 && cell.grid_position().y % i_native_chunk_size() == 0) {
				chunkbox.setPosition(gridbox.getPosition());
				chunkbox.setSize({f_cell_size() * f_native_chunk_size(), f_cell_size() * f_native_chunk_size()});
				win.draw(chunkbox);
			}
			win.draw(gridbox);
		}
	}
}

void Canvas::load(data::ResourceFinder& finder, std::string const& room_name, bool local) {

	map_states.clear();
	redo_states.clear();

	// init map_states
	map_states.push_back(Map());
	clear();

	auto const& source = local ? finder.paths.editor : finder.paths.levels;

	std::string metapath = (source / room_name / "meta.json").string();
	std::string tilepath = (source / room_name / "tile.json").string();

	data.meta = dj::Json::from_file((metapath).c_str());
	assert(!data.meta.is_null());
	data.tiles = dj::Json::from_file((tilepath).c_str());
	assert(!data.tiles.is_null());

	if (!local) { entities = EntitySet{finder, data.meta, room_name}; }

	auto const& meta = data.meta["meta"];
	room_id = meta["room_id"].as<int>();
	metagrid_coordinates.x = meta["metagrid"][0].as<int>();
	metagrid_coordinates.y = meta["metagrid"][1].as<int>();
	dimensions.x = meta["dimensions"][0].as<int>();
	dimensions.y = meta["dimensions"][1].as<int>();
	real_dimensions = {static_cast<float>(dimensions.x) * constants.cell_size, static_cast<float>(dimensions.y) * constants.cell_size};
	for (uint32_t i = 0; i < NUM_LAYERS; ++i) { map_states.back().layers.push_back(Layer(i, (i == MIDDLEGROUND), dimensions)); }

	auto style_value = meta["style"].as<int>();
	styles.tile = static_cast<Style>(style_value);
	bg = static_cast<Backdrop>(meta["background"].as<int>());
	entities.variables.music = meta["music"].as_string();
	styles.breakable = meta["styles"]["breakables"].as<int>();
	cutscene.flag = static_cast<bool>(meta["cutscene_on_entry"]["flag"].as_bool());
	cutscene.type = meta["cutscene_on_entry"]["type"].as<int>();
	cutscene.id = meta["cutscene_on_entry"]["id"].as<int>();
	cutscene.source = meta["cutscene_on_entry"]["source"].as<int>();

	background = std::make_unique<Background>(finder, meta["background"].as<int>());

    // tiles
    int layer_counter{};
    for (auto& layer : map_states.back().layers) {
        int cell_counter{};
        for (auto& cell : data.tiles["layers"][layer_counter].array_view()) {
            layer.grid.cells.at(cell_counter).value = cell.as<int>();
            layer.grid.cells.at(cell_counter).type = lookup_type(cell.as<int>());
            ++cell_counter;
        }
        ++layer_counter;
    }
    
}

bool Canvas::save(data::ResourceFinder& finder, std::string const& room_name) {

	std::filesystem::create_directory(finder.paths.levels / room_name);
	std::filesystem::create_directory(finder.paths.out / room_name);

	// clean jsons
	data = {};

	// empty json array
	constexpr auto empty_array = R"([])";
	auto const wipe = dj::Json::parse(empty_array);

	// data.meta
	data.meta["meta"]["room_id"] = room_id;
	data.meta["meta"]["metagrid"][0] = metagrid_coordinates.x;
	data.meta["meta"]["metagrid"][1] = metagrid_coordinates.y;
	data.meta["meta"]["dimensions"][0] = dimensions.x;
	data.meta["meta"]["dimensions"][1] = dimensions.y;
	data.meta["meta"]["chunk_dimensions"][0] = chunk_dimensions().x;
	data.meta["meta"]["chunk_dimensions"][1] = chunk_dimensions().y;
	data.meta["meta"]["style"] = static_cast<int>(styles.tile);
	data.meta["meta"]["background"] = static_cast<int>(bg);
	data.meta["meta"]["music"] = entities.variables.music;
	data.meta["meta"]["styles"]["breakables"] = styles.breakable;
	data.meta["meta"]["cutscene_on_entry"]["flag"] = dj::Boolean{cutscene.flag};
	data.meta["meta"]["cutscene_on_entry"]["type"] = cutscene.type;
	data.meta["meta"]["cutscene_on_entry"]["id"] = cutscene.id;
	data.meta["meta"]["cutscene_on_entry"]["source"] = cutscene.source;

	entities.save(finder, data.meta, room_name);

	data.tiles["layers"] = wipe;
	for (int i = 0; i < NUM_LAYERS; ++i) { data.tiles["layers"].push_back(wipe); }
	// push layer data
	int current_layer{};
	for (auto& layer : map_states.back().layers) {
		int current_cell{};
		for ([[maybe_unused]] auto& cell : layer.grid.cells) {
			data.tiles["layers"][current_layer].push_back(layer.grid.cells.at(current_cell).value);
			++current_cell;
		}
		++current_layer;
	}

	auto success{true};
	if (!data.meta.to_file((finder.paths.levels / room_name / "meta.json").string().c_str())) { success = false; }
	if (!data.tiles.to_file((finder.paths.levels / room_name / "tile.json").string().c_str())) { success = false; }
	if (!data.meta.to_file((finder.paths.out / room_name / "meta.json").string().c_str())) { success = false; }
	if (!data.tiles.to_file((finder.paths.out / room_name / "tile.json").string().c_str())) { success = false; }

    return success;
}

void Canvas::clear() {
    if (!map_states.empty()) {
		for (auto& layer : map_states.back().layers) { layer.erase(); }
        map_states.back().layers.clear();
    }
}

void Canvas::save_state(Tool& tool, bool force) {
    auto const& type = tool.type;
    auto undoable_tool = type == ToolType::brush || type == ToolType::fill || type == ToolType::marquee || type == ToolType::erase;;
	if ((undoable_tool && tool.clicked() && editable()) || force) {
		map_states.emplace_back(Map{map_states.back()});
        clear_redo_states();
    }
}

void Canvas::undo() {
    if (map_states.size() > 1) {
        redo_states.push_back(map_states.back());
        map_states.pop_back();
		dimensions = map_states.back().layers.back().dimensions;
    }
}

void Canvas::redo() {
    if (redo_states.size() > 0) {
        map_states.push_back(redo_states.back());
		redo_states.pop_back();
		dimensions = map_states.back().layers.back().dimensions;
    }
}

void Canvas::clear_redo_states() { redo_states.clear(); }

void Canvas::unhover() { state.reset(CanvasState::hovered); }

void Canvas::move(sf::Vector2<float> distance) { camera.move(distance); }

void Canvas::set_position(sf::Vector2<float> to_position) { camera.set_position(to_position); }

void Canvas::set_origin(sf::Vector2<float> to_origin) { origin = to_origin; }

void Canvas::set_offset_from_center(sf::Vector2<float> offset) { offset_from_center = offset; }

void Canvas::set_scale(float to_scale) { scale = to_scale; }

void Canvas::resize(sf::Vector2i adjustment) {
	// map dimensions can't be 0
	if (adjustment.x < 0 && chunk_dimensions().x == 1) { return; }
	if (adjustment.y < 0 && chunk_dimensions().y == 1) { return; }
	auto current = Map{map_states.back()};
	dimensions.x += adjustment.x * static_cast<int>(chunk_size);
	dimensions.y += adjustment.y * static_cast<int>(chunk_size);
	map_states.push_back(Map());
	for (uint32_t i = 0; i < NUM_LAYERS; ++i) {
		map_states.back().layers.push_back(Layer(i, (i == MIDDLEGROUND), dimensions));
		map_states.back().layers.back().grid.match(current.layers.at(i).grid);
	}
	clear_redo_states();
}

void Canvas::center(sf::Vector2<float> point) { set_position(point - real_dimensions * 0.5f); }

void Canvas::zoom(float amount) { scale = std::clamp(scale + amount, min_scale, max_scale); }

void Canvas::set_backdrop_color(sf::Color color) { border.setFillColor(color); }

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

TILE_TYPE Canvas::lookup_type(int idx) {
    if(idx < 1) {
        return TILE_NULL;
    }
    if(idx < 192) {
        return TILE_BASIC;
    }
    if(idx <= 223) {
        return TILE_RAMP;
    }
    if(idx <= 227) {
        return TILE_LAVA;
    }
    if(idx <= 231) {
        return TILE_CURRENT;
    }
    if(idx <= 235) {
        return TILE_FLAMMABLE;
    }
    if(idx <= 239) {
        return TILE_PLATFORM;
    }
    if(idx <= 243) {
        return TILE_WATER;
    }
    if(idx <= 247) {
        return TILE_BREAKABLE;
    }
    if(idx <= 251) {
        return TILE_LADDER;
    }
    if(idx <= 255) {
        return TILE_SPIKES;
    }
    return TILE_NULL;
}

}
