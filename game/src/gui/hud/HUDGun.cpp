
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
		auto const hotbar_size = player.hotbar.value().size();
		for (int i = 0; i < hotbar_size; ++i) {
			auto gun_index = svc.data.get_gun_id_from_tag(player.hotbar.value().get_tag(i));
			p_animatable->set_frame(gun_index);
			p_animatable->set_position({get_root().x, get_root().y - i * p_animatable->get_f_dimensions().y * constants::f_scale_factor});
			win.draw(*p_animatable);
			if (i == player.hotbar.value().get_selection()) {
				sprites.gun.setTextureRect(sf::IntRect({0, gun_index * gun_dimensions.y}, gun_dimensions));
				sprites.gun.setPosition({origins.gun.x + gun_pad_horiz, origins.gun.y - i * gun_dimensions.y * constants::f_scale_factor - i * gun_pad_vert});
				win.draw(sprites.gun);
				pointer_index = i;
			}
		}
	}
}

} // namespace fornani::gui
