
#include "fornani/automa/states/FileMenu.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Constants.hpp"

namespace fornani::automa {

FileMenu::FileMenu(ServiceProvider& svc, player::Player& player) : MenuState(svc, player, "file"), map(svc, player) {
	m_parent_menu = MenuType::play;
	current_selection = util::Circuit(num_files);
	svc.data.load_blank_save(player);
	hud.orient(svc, player, true); // display hud preview for each file in the center of the screen
	svc.state_controller.next_state = svc.data.load_progress(player, current_selection.get());
	player.set_position({svc.window->f_screen_dimensions().x / 2 + 80, 360});
	player.set_desired_direction(SimpleDirection(LR::left));
	player.antennae.at(0).set_position({svc.window->f_screen_dimensions().x / 2 + 80, 360});
	player.antennae.at(1).set_position({svc.window->f_screen_dimensions().x / 2 + 80, 360});
	player.hurt_cooldown.cancel();

	loading.start(4);
	refresh(svc);
	player.animation.force(player::AnimState::run, "run");
}

void FileMenu::tick_update(ServiceProvider& svc, capo::IEngine& engine) {
	m_input_authorized = !m_file_select_menu && !m_console;
	MenuState::tick_update(svc, engine);
	if (!m_console) {
		if (m_file_select_menu) {
			m_file_select_menu->handle_inputs(svc.controller_map, svc.soundboard);
		} else {
			if (svc.controller_map.digital_action_status(config::DigitalAction::menu_down).triggered || svc.controller_map.digital_action_status(config::DigitalAction::menu_up).triggered) {
				svc.data.load_blank_save(*player);
				svc.state_controller.next_state = svc.data.load_progress(*player, current_selection.get());
			}
		}
		if (svc.controller_map.digital_action_status(config::DigitalAction::menu_cancel).triggered) {
			if (m_file_select_menu) {
				m_file_select_menu = {};
				svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
			}
		}
		if (svc.controller_map.digital_action_status(config::DigitalAction::menu_select).triggered) {
			if (m_file_select_menu) {
				switch (m_file_select_menu->get_selection()) {
				case 0:
					svc.state_controller.next_state = svc.data.load_progress(*player, current_selection.get(), true);
					svc.state_controller.actions.set(Actions::trigger);
					svc.state_controller.actions.set(Actions::save_loaded);
					svc.soundboard.flags.menu.set(audio::Menu::select);
					svc.soundboard.flags.world.set(audio::World::load);
					break;
				case 1:
					svc.state_controller.actions.set(automa::Actions::print_stats);
					svc.state_controller.actions.set(Actions::trigger);
					svc.soundboard.flags.menu.set(audio::Menu::select);
					break;
				case 2:
					m_console = std::make_unique<gui::Console>(svc, svc.text.basic, "delete_file", gui::OutputType::gradual);
					m_file_select_menu = {};
					break;
				}
			} else {
				// TODO: pull option strings from a .json to make localization easier in the future
				m_file_select_menu = gui::MiniMenu(svc, {"play", "stats", "delete"}, options.at(current_selection.get()).position);
			}
		}
	}
	for (auto& option : options) { option.update(svc, current_selection.get()); }

	// file deletion requested
	if (svc.state_controller.actions.consume(Actions::delete_file)) {
		svc.data.delete_file(current_selection.get());
		refresh(svc);
		svc.state_controller.next_state = svc.data.load_progress(*player, current_selection.get());
	}

	auto& opt = options.at(current_selection.get());
	auto minimenu_dim{sf::Vector2f{8.f, 8.f}}; // defines the width of the nineslice, which does not include corner and edge dimensions. 8.f is enough to comfortably hold all the file options.
	auto minimenu_pos{opt.position + sf::Vector2f{opt.label.getLocalBounds().getCenter().x + minimenu_dim.x * 0.5f + 2.f * spacing, 0.f}};
	if (m_file_select_menu) { m_file_select_menu->update(svc, minimenu_dim, minimenu_pos); }

	player->animation.request(player::AnimState::run);
	player->collider.physics.zero();
	player->collider.reset();
	player->controller.autonomous_walk();
	player->collider.flags.state.set(shape::State::grounded);

	player->set_position({svc.window->i_screen_dimensions().x * 0.5f + 80, 360});
	player->update(map);
	player->controller.direction.lnr = LNR::left;

	hud.update(svc, *player);

	loading.update();

	player->controller.clean();
	player->flags.triggers = {};
}

void FileMenu::frame_update(ServiceProvider& svc) {}

void FileMenu::render(ServiceProvider& svc, sf::RenderWindow& win) {
	if (!loading.is_complete()) { return; }
	MenuState::render(svc, win);
	for (auto& option : options) { win.draw(option.label); }
	player->render(svc, win, {});
	if (loading.is_complete()) {
		hud.render(svc, *player, win);
		if (m_file_select_menu) { m_file_select_menu->render(win); }
	}
	if (m_console) {
		m_console.value()->render(win);
		m_console.value()->write(win, false);
	}
}

void FileMenu::refresh(ServiceProvider& svc) {
	auto ctr{0};
	for (auto& save : svc.data.files) {
		if (save.is_new() && options.at(ctr).label.getString().getSize() < 8) { options.at(ctr).label.setString(options.at(ctr).label.getString() + " (new)"); }
		++ctr;
	}
	for (auto& option : options) { option.update(svc, current_selection.get()); }
}

} // namespace fornani::automa
