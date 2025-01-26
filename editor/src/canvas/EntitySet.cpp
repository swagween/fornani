
#include "editor/canvas/EntitySet.hpp"
#include "editor/canvas/Canvas.hpp"
#include "fornani/setup/ResourceFinder.hpp"
#include <cassert>

namespace pi {

EntitySet::EntitySet(data::ResourceFinder& finder, dj::Json& metadata, std::string const& room_name) {

	load(finder, metadata, room_name);

	enemy_thumbnails.loadFromFile((finder.paths.editor / "enemies" / "thumbnails.png").string());

	player_box.setFillColor(sf::Color{100, 200, 100, 30});
	player_box.setOutlineColor(sf::Color{100, 200, 100, 120});
	player_box.setOutlineThickness(-2);
}

void EntitySet::render(Canvas& map, sf::RenderWindow& win, sf::Vector2<float> cam) {

	// draw player start position
	player_box.setOrigin(map.get_origin());
	player_box.setSize({map.f_cell_size(), map.f_cell_size()});
	player_box.setPosition(sf::Vector2<float>{variables.player_start} * map.f_cell_size() + cam);
	win.draw(player_box);

	// draw save point
	if (variables.save_point) {
		auto& save = variables.save_point.value();
		save->drawbox.setFillColor(sf::Color{220, 20, 220, 128});
		save->drawbox.setOutlineColor(sf::Color{240, 230, 255, 180});
		save->drawbox.setOutlineThickness(-1);
		save->drawbox.setOrigin(map.get_origin());
		save->drawbox.setSize({map.f_cell_size(), map.f_cell_size()});
		save->drawbox.setPosition({(save->position.x) * map.f_cell_size() + cam.x, (save->position.y) * map.f_cell_size() + cam.y});
		win.draw(save->drawbox);
	}

	// draw general entities
	for (auto& ent : variables.entities) { ent->render(win, cam, map.f_cell_size()); }
}

void EntitySet::load(data::ResourceFinder& finder, dj::Json& metadata, std::string const& room_name) {
	std::string inspectable_path = (finder.paths.levels / room_name / "inspectables.json").string();
	data.inspectables = dj::Json::from_file((inspectable_path).c_str());
	variables.music = metadata["music"].as_string();
	
	// load variables
	// save point
	
	//variables.save_point = std::make_unique<SavePoint>(metadata["save_point"]["id"].as<int>());
	//variables.save_point.value()->unserialize(metadata["save_point"]);

	// general entities
	for (auto const& [key, entry] : metadata["entities"].object_view()) {
		/*variables.entities.push_back(std::make_unique<Entity>(std::string{key}));
		variables.entities.back()->unserialize(entry);*/
	}
}

bool EntitySet::save(data::ResourceFinder& finder, dj::Json& metadata, std::string const& room_name) {

	// clean jsons
	data = {};

	// empty json array
	constexpr auto empty_array = R"([])";
	auto const wipe = dj::Json::parse(empty_array);

	// save point
	if (variables.save_point) { variables.save_point.value()->serialize(metadata["save_point"]); }

	// general entities
	for (auto& ent : variables.entities) {
		auto label = ent->get_label();
		auto entry = wipe;
		ent->serialize(entry);
		metadata[label].push_back(entry);
	}

	auto success{true};
	if (!metadata.to_file((finder.paths.levels / room_name / "meta.json").string().c_str())) {
		success = false;
	} else {
		std::cout << "written to file!\n";
	}
	if (!metadata.to_file((finder.paths.out / room_name / "meta.json").string().c_str())) { success = false; }
	return success;
}

void EntitySet::clear() { variables = {}; }

bool EntitySet::has_entity_at(sf::Vector2<uint32_t> pos) const {
	for (auto& s : variables.entities) {
		if (s->position.x == pos.x && s->position.y == pos.y) { return true; };
	}
	return false;
}

} // namespace pi

