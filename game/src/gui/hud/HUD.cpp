
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/hud/HUD.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani::gui {

constexpr auto f_distance_from_edge = 20.f;
constexpr auto f_pad = 4.f;

HUD::HUD(automa::ServiceProvider& svc, player::Player& player)
	: m_origin{f_distance_from_edge, svc.window->f_screen_dimensions().y - f_distance_from_edge}, hearts{svc, player, {0.f, 0.f}, {11.f, 11.f}}, gun{svc, player, "hud_gun", {}, {44, 9}}, orbs{svc, player, "hud_orb_font", {}, {9, 10}},
	  ammo{svc, player, "hud_ammo", {}, {5, 14}} {
	set_position(m_origin);
}

void HUD::update(automa::ServiceProvider& svc, player::Player& player) {
	hearts.update(svc, player);
	orbs.update(svc, player);
	ammo.update(svc, player);
	gun.update(svc, player);
}

void HUD::render(automa::ServiceProvider& svc, player::Player& player, sf::RenderWindow& win) {
	auto sign = -1.f;
	hearts.render(svc, player, win, m_position);
	auto offset = m_position + sign * hearts.get_offset();
	orbs.render(svc, player, win, offset);
	offset += sign * orbs.get_offset();
	ammo.render(svc, player, win, offset);
	offset += sign * ammo.HUDAmmo::get_offset();
	gun.render(svc, player, win, offset);
}

} // namespace fornani::gui
