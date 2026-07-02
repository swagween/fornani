
#include <fornani/automa/states/StatSheet.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/utils/Constants.hpp>

namespace fornani::automa {

StatSheet::StatSheet(ServiceProvider& svc, player::Player& player, AppContext& ctx)
	: MenuState(svc, player, ctx, "stat"), stats(svc.text.fonts.basic.font), title(svc.text.fonts.basic.font), m_player{&player}, m_items_sprite{svc, "inventory_items", {18, 18}}, m_guns_sprite{svc, "inventory_guns", {24, 24}} {
	m_parent_menu = MenuType::file_select;
	current_selection = util::Circuit(static_cast<int>(options.size()));
	auto ctr{1};
	for (auto& option : options) {
		option.update(current_selection.get());
		option.label.setLetterSpacing(1.4f);
		option.position = {svc.window->f_center_screen().x, svc.window->f_screen_dimensions().y - 60.f - ctr * 28.f};
		++ctr;
	}
	stats = options.at(0).label;
	title = options.at(0).label;
	stats.setFont(svc.text.fonts.basic.font);

	auto filedata = svc.data.get_save();

	title.setString("post-game stats");
	std::string statistics = "death count: " + std::to_string(svc.stats.player.death_count.get_count()) + "\norbs collected: " + std::to_string(svc.stats.treasure.total_orbs_collected.get_count()) +
							 "\nrooms discovered: " + std::to_string(svc.data.discovered_rooms.size()) + "\nenemies killed: " + std::to_string(svc.stats.enemy.enemies_killed.get_count()) +
							 "\nitems found: " + std::to_string(player.catalog.inventory.items_view().size()) + "\n'get bryn's gun' time: " + svc.stats.tt_formatted() + " seconds" +
							 "\nseconds played: " + std::to_string(filedata["player_data"]["stats"]["seconds_played"].as<int>());
	stats.setString(statistics);
	stats.setLineSpacing(1.f);
	svc.music_player.load(svc.finder, "firstwind");
	svc.music_player.play_looped();
	loading.start();
	player.update_wardrobe();
	for (auto [i, item] : std::views::enumerate(player.catalog.inventory.items_view())) { m_number_displays.push_back(gui::NumberDisplay(svc, player.catalog.inventory.get_quantity(item.item->get_label()), item.item->get_id())); }
	hud.update(svc, player);
}

void StatSheet::tick_update(ServiceProvider& svc, capo::IEngine& engine) {
	MenuState::tick_update(svc, engine);
	if (was_selected(svc.input_system)) {
		if (current_selection.get() == 1) {
			svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
			svc.events.return_to_main_menu_event.dispatch();
			svc.state_controller.actions.set(Actions::trigger);
		}
		if (current_selection.get() == 0) { svc.state_controller.actions.set(Actions::screenshot); }
		svc.soundboard.flags.menu.set(audio::Menu::select);
	}

	loading.update();
	auto ctr{1};
	for (auto& option : options) {
		option.update(current_selection.get());
		option.label.setLetterSpacing(1.4f);
		option.position = {svc.window->f_center_screen().x, svc.window->f_screen_dimensions().y - 40.f - ctr * 28.f};
		++ctr;
	}
	stats.setPosition({460.f, 265.f});
	title.setPosition({svc.window->f_screen_dimensions().x * 0.5f, 60.f});
}

void StatSheet::frame_update(ServiceProvider& svc) {}

void StatSheet::render(ServiceProvider& svc, sf::RenderWindow& win) {
	if (!loading.is_complete()) { return; }
	MenuState::render(svc, win);
	win.draw(stats);
	win.draw(title);
	m_player->wardrobe_widget.set_position(sf::Vector2f{40.f, 40.f});
	m_player->wardrobe_widget.render(win, {}, false);
	auto xdim = 16;
	auto fxdim = static_cast<float>(xdim);
	for (auto [i, item] : std::views::enumerate(m_player->catalog.inventory.items_view())) {
		auto where = sf::Vector2f{300.f + static_cast<float>(i % xdim) * 36.f, 100.f + 48.f * std::floor((static_cast<float>(i) / fxdim))};
		item.item->render(win, m_items_sprite.get_sprite(), where);
		for (auto& display : m_number_displays) {
			if (display.matches(item.item->get_id())) { display.render(win, where + sf::Vector2f{36.f, 36.f}); }
		}
	}
	if (m_player->arsenal) {
		for (auto [i, gun] : std::views::enumerate(m_player->arsenal->get_loadout())) {
			m_guns_sprite.set_channel(svc.data.weapon[gun->get_tag()]["metadata"]["widget_lookup"].as<int>());
			m_guns_sprite.set_position(sf::Vector2f{200.f, 40.f} + sf::Vector2f{0.f, static_cast<float>(i) * 48.f});
			win.draw(m_guns_sprite);
		}
	}
	hud.render(svc, *m_player, win);

	svc.notifications.render(win);
}

} // namespace fornani::automa
