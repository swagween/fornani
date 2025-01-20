
#include "fornani/automa/states/StatSheet.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace automa {

StatSheet::StatSheet(ServiceProvider& svc, player::Player& player, std::string_view scene, int room_number) : GameState(svc, player, scene, room_number), stats(svc.text.fonts.basic), title(svc.text.fonts.title) {
	current_selection = util::Circuit(static_cast<int>(options.size()));
	auto ctr{1};
	for (auto& option : options) {
		option.update(svc, current_selection.get());
		option.label.setLetterSpacing(1.4f);
		option.position = {svc.constants.f_center_screen.x, svc.constants.f_screen_dimensions.y - 60.f - ctr * 28.f};
		++ctr;
	}
	left_dot.set_position(options.at(0).left_offset);
	right_dot.set_position(options.at(0).right_offset);
	stats = options.at(0).label;
	title = options.at(0).label;
	title.setString("post-game stats");
	std::string statistics = "death count: " + std::to_string(svc.stats.player.death_count.get_count()) + "\norbs collected: " + std::to_string(svc.stats.treasure.total_orbs_collected.get_count()) +
							 "\nrooms discovered: " + std::to_string(svc.data.discovered_rooms.size()) + " / 25\nguns collected: " + std::to_string(player.arsenal_size()) +
							 " / 2\n items found: " + std::to_string(player.catalog.categories.inventory.items.size()) + " / 9\n'get bryn's gun' speedrun time: " + svc.stats.tt_formatted() + " seconds";
	stats.setString(statistics);
	stats.setLineSpacing(2.0f);
	svc.music.load(svc.finder, "firstwind");
	svc.music.play_looped(10);
	loading.start();
}

void StatSheet::tick_update(ServiceProvider& svc) {
	svc.controller_map.set_action_set(config::ActionSet::Menu);

	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_down).triggered) {
		current_selection.modulate(1);
		svc.soundboard.flags.menu.set(audio::Menu::shift);
	}
	if (svc.controller_map.digital_action_status(config::DigitalAction::menu_up).triggered) {
		current_selection.modulate(-1);
		svc.soundboard.flags.menu.set(audio::Menu::shift);
	}
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
		option.position = {svc.constants.f_center_screen.x, svc.constants.f_screen_dimensions.y - 60.f - ctr * 28.f};
		++ctr;
	}
	stats.setPosition({200.f, 120.f});
	title.setPosition({svc.constants.f_screen_dimensions.x * 0.5f, 60.f});
	left_dot.update(svc);
	right_dot.update(svc);
	left_dot.set_target_position(options.at(current_selection.get()).left_offset);
	right_dot.set_target_position(options.at(current_selection.get()).right_offset);
	svc.soundboard.play_sounds(svc);
}

void StatSheet::frame_update(ServiceProvider& svc) {}

void StatSheet::render(ServiceProvider& svc, sf::RenderWindow& win) {
	if (!loading.is_complete()) { return; }
	for (auto& option : options) { win.draw(option.label); }
	win.draw(stats);
	win.draw(title);
	left_dot.render(svc, win, {0, 0});
	right_dot.render(svc, win, {0, 0});
}

} // namespace automa
