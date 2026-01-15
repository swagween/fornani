
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/hud/HUDOrbs.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <ranges>

namespace fornani::gui {

HUDOrbs::HUDOrbs(automa::ServiceProvider& svc, player::Player& player, std::string_view tag, sf::Vector2f root, sf::Vector2i dimensions) : HUDWidget{svc, player, tag, root, dimensions}, m_label_width{44.f} {}

void HUDOrbs::update(automa::ServiceProvider& svc, player::Player& player) { HUDWidget::update(svc, player); }

void HUDOrbs::render(automa::ServiceProvider& svc, player::Player& player, sf::RenderWindow& win, sf::Vector2f offset) {
	HUDWidget::render(svc, player, win, offset);
	if (!p_animatable) { return; }
	m_digits = std::to_string(player.wallet.get_balance());
	int ctr{0};
	for (auto [i, digit] : std::views::enumerate(m_digits)) {
		auto index = static_cast<int>(digit - '0');
		if (index >= 0 && index < 10) {
			p_animatable->set_channel(index);
			p_animatable->set_position({get_root().x + m_label_width + (p_animatable->get_f_dimensions().x * i), get_root().y});
			win.draw(*p_animatable);
		}
		ctr++;
	}
}

} // namespace fornani::gui
