
#include "editor/canvas/EntitySet.hpp"
#include "editor/canvas/Canvas.hpp"
#include "fornani/setup/ResourceFinder.hpp"
#include "editor/canvas/entity/Inspectable.hpp"
#include "editor/canvas/entity/Platform.hpp"

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

	// draw general entities
	for (auto& ent : variables.entities) { ent->render(win, cam, map.f_cell_size()); }
}

void EntitySet::load(data::ResourceFinder& finder, dj::Json& metadata, std::string const& room_name) {
	
	variables.save_point = std::make_unique<SavePoint>(metadata["save_point"]["id"].as<int>());
	variables.save_point.value()->unserialize(metadata["save_point"]);

	// general entities
	for (auto const& [key, entry] : metadata.object_view()) {
		if (std::string{key} == "inspectables") {
			for (auto& element : entry.array_view()) {
				variables.entities.push_back(std::make_unique<Inspectable>(std::string{key}));
				variables.entities.back()->unserialize(element);
			}
		}
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

bool EntitySet::has_entity_at(sf::Vector2<uint32_t> pos, bool highlighted_only) const {
	for (auto& s : variables.entities) {
		if (s->contains_position(pos)) { return highlighted_only ? s->highlighted : true; }
	}
	return false;
}

bool EntitySet::overlaps(Entity& other) const {
	for (auto i{0u}; i < other.get_dimensions().x; ++i) {
		for (auto j{0u}; j < other.get_dimensions().y; ++j) {
			if (has_entity_at(other.get_position() + sf::Vector2u{i, j})) { return true; }
		}
	}
	return false;
}

} // namespace pi

