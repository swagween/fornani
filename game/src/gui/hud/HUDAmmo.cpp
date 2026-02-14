
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/hud/HUDAmmo.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani::gui {

HUDAmmo::HUDAmmo(automa::ServiceProvider& svc, player::Player& player, std::string_view tag, sf::Vector2f root, sf::Vector2i dimensions)
	: HUDWidget{svc, player, tag, root, dimensions}, m_reload_bar{svc, {64.f, 4.f}, {colors::blue, colors::goldenrod, colors::navy_blue}, false} {}

void HUDAmmo::update(automa::ServiceProvider& svc, player::Player& player) {
	HUDWidget::update(svc, player);
	if (!p_animatable) { return; }
	if (player.arsenal && player.hotbar) {
		if (player.switched_weapon() || Flaggable<HUDAmmoFlags>::consume_flag(HUDAmmoFlags::switched) || !m_ammo_bar) {
			m_ammo_bar = WidgetBar(svc, player.equipped_weapon().ammo.get_capacity(), p_animatable->get_dimensions(), "heads_up_display_ammo", get_root(), 2.f, true);
		}
		player.hotbar->sync();
		auto& player_ammo = player.equipped_weapon().ammo;
		if (m_ammo_bar) {
			m_reload_bar.set_origin(sf::Vector2f{0.f, m_reload_bar.get_dimensions().y});
			m_ammo_bar->set_origin(sf::Vector2f{0.f, p_animatable->get_f_dimensions().y + m_reload_bar.get_dimensions().y});
			m_ammo_bar->update(svc, player_ammo.magazine, player.equipped_weapon().shot());
			m_reload_bar.set_dimensions({player.equipped_weapon().get_reload().get_native_time() * 0.2f, 4.f});
			m_reload_bar.update(svc, m_ammo_bar->get_position(), player.equipped_weapon().get_reload().get_inverse_normalized());
		}
	}
}

void HUDAmmo::render(automa::ServiceProvider& svc, player::Player& player, sf::RenderWindow& win, sf::Vector2f offset) {
	HUDWidget::render(svc, player, win, offset);
	if (!p_animatable) { return; }
	if (player.arsenal && player.hotbar) {
		if (m_ammo_bar) { m_ammo_bar->set_position(get_root() + offset); }
		if (m_ammo_bar) { m_ammo_bar->render(win); }
		m_reload_bar.set_position(m_ammo_bar->get_position() + sf::Vector2f{0.f, -2.f});
		m_reload_bar.render(win);
	}
}

auto HUDAmmo::get_offset(bool scaled) const -> sf::Vector2f { return HUDWidget::get_offset() + sf::Vector2f{0.f, m_reload_bar.get_dimensions().y * 3.f}; }

} // namespace fornani::gui
