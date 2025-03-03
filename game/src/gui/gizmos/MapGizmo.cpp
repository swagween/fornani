
#include "fornani/gui/gizmos/MapGizmo.hpp"
#include "fornani/gui/gizmos/MapInfoGizmo.hpp"

#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Random.hpp"
#include "fornani/world/Map.hpp"

namespace fornani::gui {

MapGizmo::MapGizmo(automa::ServiceProvider& svc, world::Map& map, player::Player& player)
	: Gizmo("Minimap", false), m_minimap{std::make_unique<MiniMap>(svc)}, m_sprite{svc.assets.get_texture("map_gizmo")}, m_plugin_sprite{svc.assets.get_texture("map_gizmo")}, m_icon_sprite{svc.assets.get_texture("map_gizmo")},
	  m_map_screen(svc, svc.assets.get_texture("map_screen"), {45, 45}, {1, 1}), m_map_shadow(svc, svc.assets.get_texture("map_shadow"), {45, 45}, {1, 1}),
	  m_path{svc.finder, std::filesystem::path{"/data/gui/gizmo_paths.json"}, "minimap", 32, util::InterpolationType::quadratic},
	  m_motherboard_path{svc.finder, std::filesystem::path{"/data/gui/gizmo_paths.json"}, "minimap_motherboard", 108, util::InterpolationType::linear},
	  m_constituents{.gizmo{.top_left{.lookup{{0, 0}, {67, 55}}},
							.top_right{.lookup{{67, 0}, {63, 55}}, .position{134.f, 0.f}},
							.bottom_left{.lookup{{13, 55}, {54, 54}}, .position{26.f, 110.f}},
							.bottom_right{.lookup{{67, 55}, {61, 55}}, .position{134.f, 110.f}},
							.motherboard{.lookup{{0, 151}, {89, 126}}, .position{0.f, 0.f}}}},
	  m_lookups{.plugin{89, 173}, .icon{130, 0}}, m_chain_offsets{
													  sf::Vector2f{-35.f, 74.f},
													  sf::Vector2f{-35.f, -108.f},
													  sf::Vector2f{-100.f, -20.f},
													  sf::Vector2f{80.f, -20.f},
												  } {

	m_physics.position = sf::Vector2f{0.f, svc.constants.f_screen_dimensions.y};
	m_icon_sprite.setOrigin({3.f, 3.f});
	m_icon_sprite.setScale(svc.constants.texture_scale);
	for (auto& id : svc.data.discovered_rooms) { m_minimap->bake(svc, map, player, id, id == svc.current_room); }
	m_minimap->center();
	m_sprite.setScale(svc.constants.texture_scale);
	m_plugin_sprite.setScale(svc.constants.texture_scale);
	m_path.set_section("close");
	m_motherboard_path.set_section("start");
	m_placement = {380.f, -22.f};
}

void MapGizmo::update(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map, sf::Vector2f position) {
	Gizmo::update(svc, player, map, position);
	if (m_state == GizmoState::selected && m_switched) {
		on_open(svc, player, map);
	} else if (m_switched) {
		on_close(svc, player, map);
	}
	if (m_state == GizmoState::selected) { svc.soundboard.flags.pioneer.set(audio::Pioneer::hum); }
	if (m_path.get_section() == 0 && m_path.completed_step(1)) { svc.soundboard.flags.pioneer.set(audio::Pioneer::chain); }
	if (m_path.get_section() == 0 && m_path.completed_step(1)) { svc.soundboard.flags.pioneer.set(audio::Pioneer::boot); }
	if (m_path.get_section() == 0 && m_path.completed_step(2)) { svc.soundboard.flags.pioneer.set(audio::Pioneer::open); }
	if (m_path.get_section() == 1 && m_path.completed_step(2)) { svc.soundboard.flags.pioneer.set(audio::Pioneer::hard_slot); }

	m_path.update();
	m_motherboard_path.update();

	if (m_info) { m_info->update(svc, player, map, m_placement + m_path.get_position()); }
	for (auto& plugin : m_plugins) { plugin.update(svc.soundboard); }

	// create and destroy chain borders
	auto sparams = vfx::SpringParameters{0.94f, 0.2f, 0.f, 0.f};
	if (m_path.get_step() == 2 && m_path.get_section() == 0 && m_chains.empty()) {
		// bottom
		m_chains.push_back(
			std::make_unique<vfx::Chain>(svc, svc.assets.get_texture("map_chain"), sparams, m_path.get_position() + sf::Vector2f{0.f, m_path.get_dimensions().y} + m_placement + sf::Vector2f{-60.f, m_chain_offsets.at(0).y}, 14, false, 4.f));
		m_chains.back()->set_texture_rect(sf::IntRect{{0, 39}, {26, 32}});
		// top
		m_chains.push_back(
			std::make_unique<vfx::Chain>(svc, svc.assets.get_texture("map_chain"), sparams, m_path.get_position() + sf::Vector2f{0.f, m_path.get_dimensions().y} + m_placement + sf::Vector2f{-60.f, m_chain_offsets.at(1).y}, 14, false, 4.f));
		m_chains.back()->set_texture_rect(sf::IntRect{{0, 27}, {26, 12}});
	}
	if (m_path.get_step() == 3 && m_path.get_section() == 0 && m_chains.size() < 4) {
		// left
		m_chains.push_back(std::make_unique<vfx::Chain>(svc, svc.assets.get_texture("map_chain"), sparams, m_path.get_position() + sf::Vector2f{0.f, m_path.get_dimensions().y} + m_placement + m_chain_offsets.at(2), 6, false));
		m_chains.back()->set_texture_rect(sf::IntRect{{}, {12, 28}});
		// right
		m_chains.push_back(std::make_unique<vfx::Chain>(svc, svc.assets.get_texture("map_chain"), sparams, m_path.get_position() + m_path.get_dimensions() + m_placement + m_chain_offsets.at(3), 6, false));
		m_chains.back()->set_texture_rect(sf::IntRect{{12, 0}, {12, 28}});
	}

	if (m_path.get_step() == 1 && m_path.get_section() == 1) { m_chains.clear(); }

	// update chains
	auto ctr{0};
	for (auto& chain : m_chains) {
		if (m_chain_offsets.size() < 4) { break; }
		switch (ctr) {
		case 0:
			chain->set_position(m_path.get_position() + m_placement + sf::Vector2f{0.f, m_path.get_dimensions().y} + m_chain_offsets.at(ctr));
			chain->set_end_position(m_path.get_position() + m_path.get_dimensions() + m_placement + m_chain_offsets.at(ctr));
			chain->snap_to_axis(false);
			break;
		case 1:
			chain->set_position(m_path.get_position() + m_placement + m_chain_offsets.at(ctr));
			chain->set_end_position(m_path.get_position() + sf::Vector2f{m_path.get_dimensions().x, 0.f} + m_placement + m_chain_offsets.at(ctr));
			chain->snap_to_axis(false);
			break;
		case 2:
			chain->set_end_position(m_path.get_position() + m_placement + m_chain_offsets.at(ctr));
			chain->set_position(m_path.get_position() + sf::Vector2f{0.f, m_path.get_dimensions().y} + m_placement + m_chain_offsets.at(ctr));
			break;
		case 3:
			chain->set_position(m_path.get_position() + m_placement + m_path.get_dimensions() + m_chain_offsets.at(ctr));
			chain->set_end_position(m_path.get_position() + sf::Vector2f{m_path.get_dimensions().x, 0.f} + m_placement + m_chain_offsets.at(ctr));
			break;
		}
		auto simulations{8};
		for (auto i{0}; i < simulations; ++i) { chain->update(svc, map, player); }
		++ctr;
	}

	m_minimap->set_port_position(m_path.get_position() + m_placement - m_map_screen.get_f_corner_dimensions());
	m_minimap->set_port_dimensions(m_map_screen.get_bounds());
	if (m_state != GizmoState::selected || (m_state == GizmoState::selected && !m_path.finished())) { m_minimap->center(); }
	m_map_screen.set_position(m_path.get_position() + m_placement);
	m_map_screen.set_dimensions(m_path.get_dimensions());
	m_map_shadow.set_position(m_path.get_position() + m_placement);
	m_map_shadow.set_dimensions(m_path.get_dimensions());
}

void MapGizmo::render(automa::ServiceProvider& svc, sf::RenderWindow& win, [[maybe_unused]] player::Player& player, sf::Vector2f cam, bool foreground) {
	if (is_foreground() != foreground) { return; }
	Gizmo::render(svc, win, player, cam);
	auto render_position{-m_placement + cam};
	m_constituents.gizmo.motherboard.position = m_path.get_position() + m_motherboard_path.get_position() - m_map_screen.get_f_corner_dimensions();
	m_constituents.gizmo.motherboard.render(win, m_sprite, render_position, sf::Vector2f{100.f, -6.f});
	m_map_screen.render(win, cam);
	m_minimap->render(svc, win, player, cam, m_icon_sprite);
	m_icon_sprite.setScale(svc.constants.texture_scale);
	m_map_shadow.render(win, cam);
	for (auto& chain : m_chains) { chain->render(svc, win, cam); }
	for (auto& plugin : m_plugins) { plugin.render(win, m_plugin_sprite, cam, {}); }
	if (m_info) { m_info->render(svc, win, player, cam, foreground); }
	m_constituents.gizmo.top_left.position = m_path.get_position();
	m_constituents.gizmo.top_left.render(win, m_sprite, render_position, sf::Vector2f{66.f, 54.f});
	m_constituents.gizmo.top_right.position = m_path.get_position() + sf::Vector2f{m_path.get_dimensions().x, 0.f};
	m_constituents.gizmo.top_right.render(win, m_sprite, render_position, sf::Vector2f{1.f, 54.f});
	m_constituents.gizmo.bottom_left.position = m_path.get_position() + sf::Vector2f{0.f, m_path.get_dimensions().y};
	m_constituents.gizmo.bottom_left.render(win, m_sprite, render_position, sf::Vector2f{53.f, 1.f});
	m_constituents.gizmo.bottom_right.position = m_path.get_position() + m_path.get_dimensions();
	m_constituents.gizmo.bottom_right.render(win, m_sprite, render_position, sf::Vector2f{1.f, 1.f});
}

bool MapGizmo::handle_inputs(config::ControllerMap& controller, audio::Soundboard& soundboard) {
	auto zoom_factor{0.1f};
	if (controller.digital_action_status(config::DigitalAction::menu_up).held) {
		m_minimap->move({0.f, -1.f});
		soundboard.flags.pioneer.set(audio::Pioneer::scan);
	}
	if (controller.digital_action_status(config::DigitalAction::menu_down).held) {
		m_minimap->move({0.f, 1.f});
		soundboard.flags.pioneer.set(audio::Pioneer::scan);
	}
	if (controller.digital_action_status(config::DigitalAction::menu_left).held) {
		m_minimap->move({-1.f, 0.f});
		soundboard.flags.pioneer.set(audio::Pioneer::scan);
	}
	if (controller.digital_action_status(config::DigitalAction::menu_right).held) {
		m_minimap->move({1.f, 0.f});
		soundboard.flags.pioneer.set(audio::Pioneer::scan);
	}
	if (controller.digital_action_status(config::DigitalAction::menu_switch_left).held) {
		m_minimap->zoom(zoom_factor);
		soundboard.flags.pioneer.set(audio::Pioneer::buzz);
	} else if (controller.digital_action_status(config::DigitalAction::menu_switch_right).held) {
		m_minimap->zoom(-zoom_factor);
		soundboard.flags.pioneer.set(audio::Pioneer::buzz);
	}
	if (controller.digital_action_status(config::DigitalAction::menu_select).triggered) {
		m_minimap->center();
		soundboard.flags.pioneer.set(audio::Pioneer::click);
	}
	return Gizmo::handle_inputs(controller, soundboard);
}

void MapGizmo::on_open(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map) {
	Gizmo::on_open(svc, player, map);
	m_path.set_section("open");

	// TODO: gate plugins based on player's inventory
	m_info = std::make_unique<MapInfoGizmo>(svc, map, sf::Vector2f{374.f, -90}); // make conditional when info bar is an item
	m_plugins.push_back(MapPlugin(svc.finder, "plugin_nani", sf::IntRect{m_lookups.plugin + sf::Vector2i{0, 49}, {63, 29}}, audio::Pioneer::slot));
	m_flags.icon.set(MapIconFlags::nani);
	m_plugins.push_back(MapPlugin(svc.finder, "plugin_save", sf::IntRect{m_lookups.plugin + sf::Vector2i{27, 0}, {23, 22}}, audio::Pioneer::sync));
	m_flags.icon.set(MapIconFlags::save);
	m_plugins.push_back(MapPlugin(svc.finder, "plugin_bed", sf::IntRect{m_lookups.plugin + sf::Vector2i{0, 27}, {36, 15}}, audio::Pioneer::click));
	m_flags.icon.set(MapIconFlags::bed);
	//

	m_motherboard_path.set_section("open");
}

void MapGizmo::on_close(automa::ServiceProvider& svc, [[maybe_unused]] player::Player& player, [[maybe_unused]] world::Map& map) {
	Gizmo::on_close(svc, player, map);
	m_path.set_section("close");
	m_plugins.clear();
	m_info = {};
	m_motherboard_path.set_section("start");
}

MapPlugin::MapPlugin(data::ResourceFinder& finder, std::string_view p, sf::IntRect lookup, audio::Pioneer sound)
	: m_path(finder, std::filesystem::path{"/data/gui/gizmo_paths.json"}, p, 48, util::InterpolationType::linear), constituent{.lookup{lookup}, .position{}}, m_delay{util::Random::random_range(0, 128)}, m_sound(sound) {
	m_path.set_section("start");
	m_delay.start();
}

void MapPlugin::update(audio::Soundboard& soundboard) {
	m_delay.update();
	constituent.position = m_path.get_position();
	if (m_delay.is_complete()) { m_path.update(); }
	if (m_path.completed_step(3)) { soundboard.flags.pioneer.set(m_sound); }
}

void MapPlugin::render(sf::RenderWindow& win, sf::Sprite& sprite, sf::Vector2f cam, sf::Vector2f origin) const { constituent.render(win, sprite, cam, origin); }

} // namespace fornani::gui
