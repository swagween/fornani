
#include "editor/canvas/EntitySet.hpp"
#include "editor/canvas/Canvas.hpp"
#include "fornani/setup/ResourceFinder.hpp"

#include <cassert>

namespace pi {

EntitySet::EntitySet(fornani::automa::ServiceProvider& svc, fornani::ResourceFinder& finder, dj::Json& metadata, std::string const& room_name) {

	create_map.emplace("beds", &create_entity<Bed>);
	create_map.emplace("chests", &create_entity<Chest>);
	create_map.emplace("lights", &create_entity<Light>);
	create_map.emplace("enemies", &create_entity<Enemy>);
	create_map.emplace("portals", &create_entity<Portal>);
	create_map.emplace("platforms", &create_entity<Platform>);
	create_map.emplace("save_point", &create_entity<SavePoint>);
	create_map.emplace("timer_blocks", &create_entity<TimerBlock>);
	create_map.emplace("inspectables", &create_entity<Inspectable>);
	create_map.emplace("switch_blocks", &create_entity<SwitchBlock>);
	create_map.emplace("destructibles", &create_entity<Destructible>);
	create_map.emplace("switch_buttons", &create_entity<SwitchButton>);

	load(svc, finder, metadata, room_name);

	player_box.setFillColor(sf::Color{100, 200, 100, 30});
	player_box.setOutlineColor(sf::Color{100, 200, 100, 120});
	player_box.setOutlineThickness(-2);
}

void EntitySet::render(Canvas& map, sf::RenderWindow& win, sf::Vector2f cam) {

	// draw player start position
	player_box.setOrigin(map.get_origin());
	player_box.setSize({map.f_cell_size(), map.f_cell_size()});
	player_box.setPosition(sf::Vector2f{variables.player_start} * map.f_cell_size() + cam);
	win.draw(player_box);

	// draw general entities
	for (auto& ent : variables.entities) { ent->render(win, cam, map.f_cell_size()); }
}

void EntitySet::load(fornani::automa::ServiceProvider& svc, fornani::ResourceFinder& finder, dj::Json& metadata, std::string const& room_name) {
	for (auto const& [key, entry] : metadata.as_object()) {
		for (auto const& element : entry.as_array()) {
			if (create_map.contains(std::string{key})) { variables.entities.push_back(create_map[key](svc, element)); }
		}
	}
}

bool EntitySet::save(fornani::ResourceFinder& finder, dj::Json& metadata, std::string const& room_name) {

	if (variables.entities.empty()) { return true; }

	auto const& wipe = dj::Json::empty_array();

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
