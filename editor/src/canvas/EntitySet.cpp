
#include "editor/canvas/EntitySet.hpp"
#include "editor/canvas/Canvas.hpp"
#include "fornani/setup/ResourceFinder.hpp"
#include "editor/canvas/entity/Inspectable.hpp"
#include "editor/canvas/entity/Platform.hpp"
#include "editor/canvas/entity/Portal.hpp"
#include "editor/canvas/entity/Enemy.hpp"
#include "editor/canvas/entity/SavePoint.hpp"

#include <cassert>

namespace pi {

EntitySet::EntitySet(fornani::data::ResourceFinder& finder, dj::Json& metadata, std::string const& room_name) {

	load(finder, metadata, room_name);

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

void EntitySet::load(fornani::data::ResourceFinder& finder, dj::Json& metadata, std::string const& room_name) {

	// general entities
	for (auto const& [key, entry] : metadata.as_object()) {
		if (std::string{key} == "inspectables") {
			for (auto element : entry.as_array()) {
				variables.entities.push_back(std::make_unique<Inspectable>());
				variables.entities.back()->unserialize(element);
			}
		}
		if (std::string{key} == "platforms") {
			for (auto element : entry.as_array()) {
				variables.entities.push_back(std::make_unique<Platform>());
				variables.entities.back()->unserialize(element);
			}
		}
		if (std::string{key} == "portals") {
			for (auto element : entry.as_array()) {
				variables.entities.push_back(std::make_unique<Portal>());
				variables.entities.back()->unserialize(element);
			}
		}
		if (std::string{key} == "enemies") {
			for (auto element : entry.as_array()) {
				variables.entities.push_back(std::make_unique<Enemy>());
				variables.entities.back()->unserialize(element);
			}
		}
		if (std::string{key} == "save_point") {
			variables.entities.push_back(std::make_unique<SavePoint>());
			variables.entities.back()->unserialize(entry);
		}
	}
}

bool EntitySet::save(fornani::data::ResourceFinder& finder, dj::Json& metadata, std::string const& room_name) {

	if (variables.entities.empty()) { return true; }

	// clean jsons
	data = {};

	auto wipe = dj::Json::empty_array();

	// general entities
	for (auto& ent : variables.entities) {
		auto label = ent->get_label();
		if (ent->unique) {
			ent->serialize(metadata[label]);
			NANI_LOG_INFO(m_logger, "Serialized unique entity with label <{}>", label);
		} else {
			auto entry = wipe;
			ent->serialize(entry);
			metadata[label].push_back(entry);
			NANI_LOG_INFO(m_logger, "Serialized unique entity with label <{}>", label);
		}
	}

	if (!metadata.to_file((finder.paths.levels / room_name).string().c_str())) { return false; }
	return true;
}

void EntitySet::clear() { variables = {}; }

bool EntitySet::has_entity_at(sf::Vector2<std::uint32_t> pos, bool highlighted_only) const {
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

