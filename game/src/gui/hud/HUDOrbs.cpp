
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/hud/HUDOrbs.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <ranges>

namespace fornani::gui {

HUDOrbs::HUDOrbs(automa::ServiceProvider& svc, player::Player& player, std::string_view tag, sf::Vector2f root, sf::Vector2i dimensions)
	: HUDWidget{svc, player, tag, root, dimensions}, m_label_width{44.f}, m_orb_text{svc, "heads_up_display_orb_font"} {}

void HUDOrbs::update(automa::ServiceProvider& svc, player::Player& player) { HUDWidget::update(svc, player); }

void HUDOrbs::render(automa::ServiceProvider& svc, player::Player& player, sf::RenderWindow& win, sf::Vector2f offset) {
	HUDWidget::render(svc, player, win, offset);
	if (!p_animatable) { return; }
	m_orb_text.set_texture_rect(sf::IntRect{{90, 0}, {22, 10}});
	m_orb_text.set_position(sf::Vector2f{get_root()} + offset);
	m_orb_text.set_origin({0.f, 10.f});
	win.draw(m_orb_text);
	m_digits = std::to_string(player.wallet.get_balance());
	for (auto [i, digit] : std::views::enumerate(m_digits)) {
		auto index = static_cast<int>(digit - '0');
		if (index >= 0 && index < 10) {
			p_animatable->set_texture_rect(sf::IntRect{{index * p_animatable->get_dimensions().x, 0}, p_animatable->get_dimensions()});
			p_animatable->set_position(sf::Vector2f{get_root().x + m_label_width + 4.f + (p_animatable->get_f_dimensions().x * i * constants::f_scale_factor), get_root().y} + offset);
			win.draw(*p_animatable);
		}
	}
}

} // namespace fornani::gui
