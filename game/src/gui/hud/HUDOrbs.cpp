
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/hud/HUDOrbs.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <ranges>

namespace fornani::gui {

HUDOrbs::HUDOrbs(automa::ServiceProvider& svc, player::Player& player, std::string_view tag, sf::Vector2f root, sf::Vector2i dimensions)
	: HUDWidget{svc, player, tag, root, dimensions}, m_label_width{44.f}, m_orb_text{svc, "heads_up_display_orb_font"}, m_displayed_amount{static_cast<float>(player.wallet.get_balance())}, m_circuit{2} {}

void HUDOrbs::update(automa::ServiceProvider& svc, player::Player& player) {
	HUDWidget::update(svc, player);
	constexpr float speed = 2.0f;
	auto amount = player.wallet.get_balance();
	m_displayed_amount += (static_cast<float>(amount) - m_displayed_amount) * speed * svc.ticker.dt.count();
	if (std::abs(m_displayed_amount - amount) < 0.01f) { m_displayed_amount = static_cast<float>(amount); }
	auto display = static_cast<int>(std::round(m_displayed_amount));
	if (display != amount) { /*svc.soundboard.play_sound("drop_spawn");*/
		m_frame = 1 + m_circuit.get();
	} else {
		m_frame = 0;
	}
	if (svc.ticker.every_x_ticks(16)) { m_circuit.modulate(1); }
	m_digits = std::to_string(display);
}

void HUDOrbs::render(automa::ServiceProvider& svc, player::Player& player, sf::RenderWindow& win, sf::Vector2f offset) {
	HUDWidget::render(svc, player, win, offset);
	if (!p_animatable) { return; }
	m_orb_text.set_texture_rect(sf::IntRect{{90, 10 * m_frame}, {22, 10}});
	m_orb_text.set_position(sf::Vector2f{get_root()} + offset);
	m_orb_text.set_origin({0.f, 10.f});
	win.draw(m_orb_text);
	for (auto [i, digit] : std::views::enumerate(m_digits)) {
		auto index = static_cast<int>(digit - '0');
		if (index >= 0 && index < 10) {
			p_animatable->set_texture_rect(sf::IntRect{{index * p_animatable->get_dimensions().x, 10 * m_frame}, p_animatable->get_dimensions()});
			p_animatable->set_position(sf::Vector2f{get_root().x + m_label_width + 4.f + (p_animatable->get_f_dimensions().x * i * constants::f_scale_factor), get_root().y} + offset);
			win.draw(*p_animatable);
		}
	}
}

} // namespace fornani::gui
