
#include <fornani/gui/dialogs/BuilderDialog.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani::gui {

fornani::gui::BuilderDialog::BuilderDialog(automa::ServiceProvider& svc, world::Map& map, player::Player& player, int vendor_id) : IDialog(svc, map, player, vendor_id, "builder") {}

void BuilderDialog::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player, SceneContext& context) {
	IDialog::update(svc, map, player, context);
	if (early_tick_return()) { return; }

	auto& controller = svc.input_system;
	if (controller.menu_move(input::MoveDirection::up)) { svc.soundboard.play_sound("menu_shift"); }
	if (controller.menu_move(input::MoveDirection::down)) { svc.soundboard.play_sound("menu_shift"); }
	if (controller.menu_move(input::MoveDirection::left)) { svc.soundboard.play_sound("menu_shift"); }
	if (controller.menu_move(input::MoveDirection::right)) { svc.soundboard.play_sound("menu_shift"); }
	if (svc.input_system.digital(input::DigitalAction::menu_tab_left).triggered) {
		p_state = is_buying() ? DialogState::sell : DialogState::buy;
		svc.soundboard.flags.menu.set(audio::Menu::select);
	}
	if (svc.input_system.digital(input::DigitalAction::menu_tab_right).triggered) {
		p_state = is_buying() ? DialogState::sell : DialogState::buy;
		svc.soundboard.flags.menu.set(audio::Menu::select);
	}
	if (svc.input_system.digital(input::DigitalAction::menu_back).triggered) {
		close();
		svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
	}
}

void BuilderDialog::render(automa::ServiceProvider& svc, sf::RenderWindow& win, player::Player& player, world::Map& map, LightShader& shader) {
	IDialog::render(svc, win, player, map, shader);
	if (early_render_return()) { return; }
}

void BuilderDialog::refresh(automa::ServiceProvider& svc, player::Player& player, world::Map& map) {}

} // namespace fornani::gui
