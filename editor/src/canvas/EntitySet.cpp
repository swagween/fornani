
#include "editor/canvas/EntitySet.hpp"
#include <cassert>
#include "editor/canvas/Canvas.hpp"
#include "editor/setup/ResourceFinder.hpp"

namespace pi {

EntitySet::EntitySet(ResourceFinder& finder, dj::Json& metadata, std::string const& room_name) {

	load(finder, metadata, room_name);

	large_animator_textures.loadFromFile((finder.paths.resources / "image" / "animators" / "large_animators_01.png").string());
	large_animator_thumbs.loadFromFile((finder.paths.resources / "image" / "animators" / "large_animator_thumbs.png").string());
	small_animator_textures.loadFromFile((finder.paths.resources / "image" / "animators" / "small_animators_01.png").string());
	enemy_thumbnails.loadFromFile((finder.paths.local / "enemies" / "thumbnails.png").string());

	sprites.current_enemy.setTexture(enemy_thumbnails);
	sprites.enemy_thumb.setTexture(enemy_thumbnails);
	sprites.large_animator.setTexture(large_animator_textures);
	sprites.large_animator_thumb.setTexture(large_animator_thumbs);
	sprites.small_animator.setTexture(small_animator_textures);
	sprites.small_animator_thumb.setTexture(small_animator_thumbs);

	player_box.setFillColor(sf::Color{100, 200, 100, 10});
	player_box.setOutlineColor(sf::Color{100, 200, 100, 70});
	player_box.setOutlineThickness(-2);

	portalbox.setFillColor(sf::Color{120, 220, 200, 128});
	portalbox.setOutlineColor(sf::Color{240, 230, 255, 180});
	portalbox.setOutlineThickness(-1);

	chestbox.setFillColor(sf::Color{220, 220, 80, 128});
	chestbox.setOutlineColor(sf::Color{40, 30, 255, 180});
	chestbox.setOutlineThickness(-3);

	savebox.setFillColor(sf::Color{220, 20, 220, 128});
	savebox.setOutlineColor(sf::Color{240, 230, 255, 180});
	savebox.setOutlineThickness(-1);

	inspbox.setFillColor(sf::Color{220, 120, 100, 128});
	inspbox.setOutlineColor(sf::Color{240, 230, 255, 180});
	inspbox.setOutlineThickness(-1);

	vinebox.setOutlineColor(sf::Color{240, 230, 80, 80});
	vinebox.setOutlineThickness(-1);

	scenerybox.setOutlineColor(sf::Color{20, 20, 180, 30});
	scenerybox.setOutlineThickness(-1);

	platextent.setFillColor(sf::Color::Transparent);
	platextent.setOutlineColor(sf::Color{240, 230, 55, 80});
	platextent.setOutlineThickness(-2);
	platbox.setFillColor(sf::Color{220, 120, 100, 128});
	platbox.setOutlineColor(sf::Color{240, 230, 255, 180});
	platbox.setOutlineThickness(-4);
}

void EntitySet::render(Canvas& map, sf::RenderWindow& win, sf::Vector2<float> cam) {

	//scale and orient boxes for zooming
	scenerybox.setOrigin(map.get_origin());
	platextent.setOrigin(map.get_origin());
	player_box.setOrigin(map.get_origin());
	portalbox.setOrigin(map.get_origin());
	chestbox.setOrigin(map.get_origin());
	savebox.setOrigin(map.get_origin());
	//inspbox.setOrigin(map.get_origin());
	vinebox.setOrigin(map.get_origin());
	scenerybox.setSize({map.f_cell_size(), map.f_cell_size()});
	platextent.setSize({map.f_cell_size(), map.f_cell_size()});
	player_box.setSize({map.f_cell_size(), map.f_cell_size()});
	portalbox.setSize({map.f_cell_size(), map.f_cell_size()});
	chestbox.setSize({map.f_cell_size(), map.f_cell_size()});
	savebox.setSize({map.f_cell_size(), map.f_cell_size()});
	inspbox.setSize({map.f_cell_size(), map.f_cell_size()});
	platbox.setSize({map.f_cell_size(), map.f_cell_size()});
	vinebox.setSize({map.f_cell_size(), map.f_cell_size()});

	for (auto& ent : variables.entities) {
		ent->drawbox.setFillColor(sf::Color{255, 60, 60, 120});
		ent->drawbox.setOutlineColor(sf::Color{60, 255, 60, 120});
		ent->drawbox.setOutlineThickness(-2.f);
		ent->drawbox.setSize(ent->f_dimensions() * map.f_cell_size());
		ent->drawbox.setPosition((ent->position.x) * map.f_cell_size() + cam.x, (ent->position.y) * map.f_cell_size() + cam.y);
		win.draw(ent->drawbox);
	}

	// player start
	player_box.setPosition(static_cast<sf::Vector2<float>>(variables.player_start) * (float)map.f_cell_size() + cam);
	win.draw(player_box);
}

void EntitySet::load(ResourceFinder& finder, dj::Json& metadata, std::string const& room_name) {
	std::string inspectable_path = (finder.paths.levels / room_name / "inspectables.json").string();
	data.inspectables = dj::Json::from_file((inspectable_path).c_str());
	variables.music = metadata["music"].as_string();
}

bool EntitySet::save(ResourceFinder& finder, dj::Json& metadata, std::string const& room_name) {

	// clean jsons
	data = {};

	// empty json array
	constexpr auto empty_array = R"([])";
	auto const wipe = dj::Json::parse(empty_array);

	int ctr{};
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
	if (!data.inspectables.to_file((finder.paths.levels / room_name / "inspectables.json").string().c_str())) { success = false; }
	if (!metadata.to_file((finder.paths.out / room_name / "meta.json").string().c_str())) { success = false; }
	if (!data.inspectables.to_file((finder.paths.out / room_name / "inspectables.json").string().c_str())) { success = false; }
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

