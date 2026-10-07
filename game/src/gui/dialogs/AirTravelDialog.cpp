
#include <fornani/entities/player/Player.hpp>
#include <fornani/graphics/Color.hpp>
#include <fornani/gui/dialogs/AirTravelDialog.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <span>

namespace fornani::gui {

fornani::gui::AirTravelDialog::AirTravelDialog(automa::ServiceProvider& svc, world::Map& map, player::Player& player, int vendor_id)
	: IDialog(svc, map, player, vendor_id, "air_travel"), m_flat_shader{svc.finder}, m_backdrop{svc, "air_travel_backdrop"}, m_marker{svc, "landing_point_marker", {16, 16}}, m_indicator{svc, "corner_selector", {32, 32}} {
	auto const& in = svc.data.travel["air_travel_dialog"];
	for (auto const& location : in["locations"].as_array()) {
		m_destinations.push_back(LandingPoint{{svc.text.fonts.title.font}, {location["position"][0].as<float>(), location["position"][1].as<float>()}});
		auto& t = m_destinations.back().tag;
		t.setString(location["tag"].as_string());
		t.setCharacterSize(svc.text.fonts.title.glyph_size);
	};
	m_selector.emplace(sf::Vector2i{1, static_cast<int>(m_destinations.size())}, sf::Vector2f{32.f, 32.f});
	m_marker.push_and_set_animation("basic", {0, 1, 20, -1});
	m_marker.center();
	m_indicator.center();
	m_indicator.push_and_set_animation("basic", {0, 6, 32, -1});
}

void AirTravelDialog::update(automa::ServiceProvider& svc, world::Map& map, player::Player& player, SceneContext& context) {
	IDialog::update(svc, map, player, context);
	if (early_tick_return()) { return; }

	auto& controller = svc.input_system;

	if (!m_selector) { return; }

	m_marker.tick();
	m_indicator.tick();

	if (controller.menu_move(input::MoveDirection::up)) {
		svc.soundboard.play_sound("menu_shift");
		if (m_selector->move_direction({0, -1}).up()) {}
	}
	if (controller.menu_move(input::MoveDirection::down)) {
		svc.soundboard.play_sound("menu_shift");
		if (m_selector->move_direction({0, 1}).down()) {}
	}
	if (controller.menu_move(input::MoveDirection::left)) {
		svc.soundboard.play_sound("menu_shift");
		if (m_selector->move_direction({-1, 0}).left()) {}
	}
	if (controller.menu_move(input::MoveDirection::right)) {
		svc.soundboard.play_sound("menu_shift");
		if (m_selector->move_direction({1, 0}).right()) {}
	}
	if (svc.input_system.digital(input::DigitalAction::menu_tab_left).triggered) {
		p_state = is_buying() ? DialogState::sell : DialogState::buy;
		svc.soundboard.flags.menu.set(audio::Menu::select);
	}
	if (svc.input_system.digital(input::DigitalAction::menu_tab_right).triggered) {
		p_state = is_buying() ? DialogState::sell : DialogState::buy;
		svc.soundboard.flags.menu.set(audio::Menu::select);
	}
	if (svc.input_system.digital(input::DigitalAction::menu_select).triggered) {}
	if (svc.input_system.digital(input::DigitalAction::menu_back).triggered) {
		close();
		svc.soundboard.flags.menu.set(audio::Menu::backward_switch);
		svc.events.set_cutscene_progression_event.dispatch(20);
	}

	if (m_selector) {
		m_selector->set_position(m_selector->get_menu_position() + p_position);
		m_selector->update();
	}
}

void AirTravelDialog::render(automa::ServiceProvider& svc, sf::RenderWindow& win, player::Player& player, world::Map& map, LightShader& shader, Renderer& renderer) {
	IDialog::render(svc, win, player, map, shader, renderer);
	if (early_render_return()) { return; }

	m_backdrop.set_position(p_position);
	win.draw(m_backdrop);

	for (auto const [i, dest] : std::views::enumerate(m_destinations)) {
		m_selector->matches(i) ? dest.flags.set(LandingPointFlags::selected) : dest.flags.reset(LandingPointFlags::selected);
		if (m_selector->matches(i)) { m_indicator.set_position(dest.position * constants::f_scale_factor); }
		m_marker.set_position(dest.position * constants::f_scale_factor);
		m_marker.set_frame(0);
		if (dest.flags.test(LandingPointFlags::selected)) { m_marker.set_frame(1); }
		win.draw(m_marker);
		auto const selected = m_selector->matches(i) ? 6.f : 0.f;
		dest.tag.setPosition({680.f - selected, 80.f + i * 20.f});
		dest.flags.test(LandingPointFlags::selected) ? dest.tag.setFillColor(colors::pioneer_red) : dest.tag.setFillColor(colors::pioneer_dark_red);
		win.draw(dest.tag);
	}

	if (m_selector) { m_selector->render(win, p_selector_sprite.get_sprite(), {2.f, 2.f}, {}); }
	win.draw(m_indicator);

	IDialog::post_render(svc, win, renderer);
}

void AirTravelDialog::debug() {
	ImGui::SetNextWindowSize(ImVec2{256.f, 128.f});
	if (ImGui::Begin("Builder Debug")) {
		if (m_selector) {
			ImGui::Text("Selection: %i", m_selector->get_current_selection());
			ImGui::Text("Selector X Index: %i", m_selector->get_index().x);
			ImGui::Text("Selector Y Index: %i", m_selector->get_index().y);
		}
		ImGui::End();
	}
}

} // namespace fornani::gui
