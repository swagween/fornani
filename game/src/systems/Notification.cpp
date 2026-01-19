
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/systems/Notification.hpp>

namespace fornani {

Notification::Notification(automa::ServiceProvider& svc, std::string_view message)
	: m_message{svc.text.fonts.basic, std::string{message}}, m_path{svc.finder, std::filesystem::path{"/data/gui/console_paths.json"}, "notification", 64, util::InterpolationType::linear}, m_stall{360}, m_fade{32}, m_input_code{"    "} {
	m_message.setCharacterSize(16);
	m_path.set_section("in");
	m_message.setPosition({24.f, -256.f});
}

void Notification::update(automa::ServiceProvider& svc) {
	m_stall.update();
	m_fade.update();
	m_path.update();
	if (m_path.completed_step(1)) { svc.soundboard.flags.console.set(audio::Console::notification); }
	if (m_path.completed_step(2)) { m_stall.start(); }
	if (m_stall.is_almost_complete()) { m_fade.start(); }
}

void Notification::render(sf::RenderWindow& win, float y_offset) {
	auto position = m_path.get_position() + sf::Vector2f{0.f, y_offset};
	m_message.setPosition(position + sf::Vector2f{2.f, 2.f});
	m_message.setFillColor(colors::ui_black);
	win.draw(m_message);
	m_message.setPosition(position);
	m_message.setFillColor(colors::ui_white);
	if (m_fade.running()) { m_fade.halfway() ? m_message.setFillColor(colors::dark_goldenrod) : m_message.setFillColor(colors::goldenrod); }
	win.draw(m_message);
	if (m_input_icon) {
		auto insertion_index = m_message.getString().find("    ");
		auto insertion_point = m_message.findCharacterPos(insertion_index);
		m_input_icon->set_position(insertion_point);
		win.draw(*m_input_icon);
	}
}

void Notification::insert_input_hint(automa::ServiceProvider& svc, int action_id, int index) {
	m_input_icon = Drawable{svc, "controller_button_icons"};
	auto lookup = svc.controller_map.get_icon_lookup_by_action(static_cast<config::DigitalAction>(action_id));
	if (!m_input_icon) { return; }
	m_input_icon->set_texture_rect(sf::IntRect{lookup * 18, {18, 18}});
	m_input_icon->set_origin({-2.f, 4.f});
}

} // namespace fornani
