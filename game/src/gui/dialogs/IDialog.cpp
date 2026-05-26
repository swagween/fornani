
#include <fornani/automa/SceneContext.hpp>
#include <fornani/graphics/Transition.hpp>
#include <fornani/gui/dialogs/IDialog.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/shader/LightShader.hpp>
#include <fornani/utils/ColorUtils.hpp>

namespace fornani::gui {

IDialog::IDialog(automa::ServiceProvider& svc, world::Map& map, player::Player& player, int vendor_id, std::string const& type)
	: m_vendor_id{vendor_id}, m_intro{300}, m_fade_in{120}, m_outro{100}, p_vendor_portrait{svc, "character_portraits"}, p_palette{"pioneer", svc.finder}, p_selector_sprite{svc, "vendor_gizmo"}, p_theme{svc.data.menu_themes["mini_white"]} {
	m_intro.start();
	p_vendor_portrait.set_texture_rect(sf::IntRect{{vendor_id * 64, 0}, {64, 128}});
	// background color
	m_background.setFillColor(colors::pioneer_black);
	m_background.setSize(svc.window->f_screen_dimensions());
	p_flags.set(DialogStatus::opened);
	util::ColorUtils::reset();

	auto npc = svc.data.get_npc_label_from_id(vendor_id);
	if (npc) {
		p_artwork.emplace(svc, type + "_" + std::string{*npc});
		p_artwork->center();
		p_artwork->set_position(svc.window->f_center_screen());
		p_npc_label = *npc;
	}

	m_helptext.emplace(svc, svc.data.gui_text["dialog"]["enter_start"].as_string(), fornani::input::DigitalAction::menu_select, svc.data.gui_text["dialog"]["enter_end"].as_string(), 195, true);
}

void IDialog::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player, SceneContext& context) {
	if (m_helptext) { m_helptext->update(); }
	if (fade_logic(svc, context.transition)) { m_flags.set(IDialogFlags::early_tick_return); }
}

void IDialog::render(automa::ServiceProvider& svc, sf::RenderWindow& win, player::Player& player, world::Map& map, LightShader& shader) {
	if (!is_closing()) {
		if (p_artwork) { win.draw(*p_artwork); }
	}
	if (is_opening() || p_flags.test(DialogStatus::waiting_to_enter)) {
		if (m_intro.is_complete() && m_helptext) { m_helptext->render(win); }
		m_flags.set(IDialogFlags::early_render_return);
		return;
	}
	if (m_fade_in.running()) { shader.set_darken(std::floor(m_fade_in.get_normalized() * 4.f)); }
	if (is_closing()) { shader.set_darken(std::floor(m_outro.get_inverse_normalized() * 4.f)); }
	win.draw(m_background);
	if (m_outro.is_complete() && p_flags.test(DialogStatus::closed)) { m_flags.set(IDialogFlags::early_render_return); }
}

void IDialog::close() { m_outro.start(); }

void IDialog::refresh(automa::ServiceProvider& svc, player::Player& player, world::Map& map) {}

bool IDialog::fade_logic(automa::ServiceProvider& svc, graphics::Transition& transition) {
	m_intro.update();
	m_outro.update();
	m_fade_in.update();
	if (m_intro.is_almost_complete()) { p_flags.set(DialogStatus::waiting_to_enter); }
	if (p_flags.test(DialogStatus::waiting_to_enter)) {
		if (svc.input_system.digital(input::DigitalAction::menu_select).triggered) {
			svc.soundboard.flags.menu.set(audio::Menu::select);
			p_flags.set(DialogStatus::entered);
		}
		/*if (svc.input_system.digital(input::DigitalAction::menu_back).triggered) {
			svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
			close();
		}*/
	}
	if (p_flags.test(DialogStatus::waiting_to_enter) && p_flags.test(DialogStatus::entered)) {
		transition.start();
		p_flags.set(DialogStatus::intro_done);
		p_flags.reset(DialogStatus::waiting_to_enter);
		p_flags.reset(DialogStatus::entered);
	}
	if (transition.is(graphics::TransitionState::black) && p_flags.test(DialogStatus::intro_done)) {
		m_fade_in.start();
		p_flags.reset(DialogStatus::intro_done);
		transition.end();
		m_helptext.reset();
	}
	if (m_outro.is_almost_complete()) {
		transition.start();
		p_flags.set(DialogStatus::closed);
	}
	if (transition.is(graphics::TransitionState::black) && p_flags.test(DialogStatus::closed)) {
		p_flags.reset(DialogStatus::opened);
		transition.end();
	}
	!p_flags.test(DialogStatus::opened) ? m_background.setFillColor(util::ColorUtils::fade_out(colors::pioneer_black)) : m_background.setFillColor(util::ColorUtils::fade_in(colors::pioneer_black));
	return is_opening() || is_closing() || p_flags.test(DialogStatus::waiting_to_enter);
}

} // namespace fornani::gui
