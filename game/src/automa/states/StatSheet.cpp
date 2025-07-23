
#include "fornani/automa/states/StatSheet.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Constants.hpp"

namespace fornani::automa {

StatSheet::StatSheet(ServiceProvider& svc, player::Player& player) : MenuState(svc, player, "stat"), stats(svc.text.fonts.basic), title(svc.text.fonts.title) {
	m_parent_menu = MenuType::file_select;
	current_selection = util::Circuit(static_cast<int>(options.size()));
	auto ctr{1};
	for (auto& option : options) {
		option.update(svc, current_selection.get());
		option.label.setLetterSpacing(1.4f);
		option.position = {svc.window->f_center_screen().x, svc.window->f_screen_dimensions().y - 60.f - ctr * 28.f};
		++ctr;
	}
	stats = options.at(0).label;
	title = options.at(0).label;
	title.setString("post-game stats");
	std::string statistics = "death count: " + std::to_string(svc.stats.player.death_count.get_count()) + "\norbs collected: " + std::to_string(svc.stats.treasure.total_orbs_collected.get_count()) +
							 "\nrooms discovered: " + std::to_string(svc.data.discovered_rooms.size()) + " / 25\nguns collected: " + std::to_string(player.arsenal_size()) +
							 " / 2\n items found: " + std::to_string(player.catalog.inventory.items_view().size()) + " / 9\n'get bryn's gun' speedrun time: " + svc.stats.tt_formatted() + " seconds";
	stats.setString(statistics);
	stats.setLineSpacing(2.0f);
	svc.music_player.load(svc.finder, "firstwind");
	svc.music_player.play_looped();
	loading.start();
}

void StatSheet::tick_update(ServiceProvider& svc, capo::IEngine& engine) {
	MenuState::tick_update(svc, engine);
	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_select).triggered) {
		if (current_selection.get() == 1) {
			svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
			svc.state_controller.actions.set(Actions::main_menu);
			svc.state_controller.actions.set(Actions::trigger);
		}
		if (current_selection.get() == 0) { svc.state_controller.actions.set(Actions::screenshot); }
		svc.soundboard.flags.menu.set(audio::Menu::select);
	}

	loading.update();
	auto ctr{1};
	for (auto& option : options) {
		option.update(svc, current_selection.get());
		option.label.setLetterSpacing(1.4f);
		option.position = {svc.window->f_center_screen().x, svc.window->f_screen_dimensions().y - 60.f - ctr * 28.f};
		++ctr;
	}
	stats.setPosition({200.f, 120.f});
	title.setPosition({svc.window->f_screen_dimensions().x * 0.5f, 60.f});
}

void StatSheet::frame_update(ServiceProvider& svc) {}

void StatSheet::render(ServiceProvider& svc, sf::RenderWindow& win) {
	if (!loading.is_complete()) { return; }
	MenuState::render(svc, win);
	win.draw(stats);
	win.draw(title);
}

} // namespace fornani::automa
