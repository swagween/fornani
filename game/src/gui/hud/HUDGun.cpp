
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/hud/HUDGun.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani::gui {

HUDGun::HUDGun(automa::ServiceProvider& svc, player::Player& player, std::string_view tag, sf::Vector2f root, sf::Vector2i dimensions) : HUDWidget{svc, player, tag, root, dimensions} {}

void HUDGun::update(automa::ServiceProvider& svc, player::Player& player) { HUDWidget::update(svc, player); }

void HUDGun::render(automa::ServiceProvider& svc, player::Player& player, sf::RenderWindow& win, sf::Vector2f offset) {
	HUDWidget::render(svc, player, win, offset);
	if (!p_animatable) { return; }
	if (player.hotbar && player.arsenal) {
		auto pointer_index{0};
		auto pad = 2.f;
		auto const hotbar_size = player.hotbar.value().size();
		for (int i = 0; i < hotbar_size; ++i) {
			auto gun_index = svc.data.get_gun_id_from_tag(player.hotbar.value().get_tag(i));
			auto pointer_exclusion = i == player.hotbar.value().get_selection() ? 0 : 10;
			auto y_pos = get_root().y - (i * p_animatable->get_f_dimensions().y * constants::f_scale_factor) - i * pad;
			p_animatable->set_texture_rect(sf::IntRect{{p_animatable->get_dimensions().x + pointer_exclusion, p_animatable->get_dimensions().y * gun_index}, p_animatable->get_dimensions()});
			p_animatable->set_position(sf::Vector2f{get_root().x + pointer_exclusion * constants::f_scale_factor, y_pos} + offset);
			win.draw(*p_animatable);
			if (i == player.hotbar.value().get_selection()) {
				p_animatable->set_texture_rect(sf::IntRect{{0, p_animatable->get_dimensions().y * gun_index}, p_animatable->get_dimensions()});
				p_animatable->set_position(sf::Vector2f{get_root().x - 2.f, y_pos} + offset);
				win.draw(*p_animatable);
				pointer_index = i;
			}
		}
	}
}

} // namespace fornani::gui
