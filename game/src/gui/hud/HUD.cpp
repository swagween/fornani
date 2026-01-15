
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/hud/HUD.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani::gui {

constexpr auto f_distance_from_edge = 20.f;
constexpr auto f_pad = 4.f;

HUD::HUD(automa::ServiceProvider& svc, player::Player& player)
	: m_origin{f_distance_from_edge, f_distance_from_edge}, hearts{svc, player, {f_distance_from_edge, svc.window->f_screen_dimensions().y - f_distance_from_edge}, {11, 11}},
	  gun{svc, player, "hud_gun", {f_distance_from_edge, svc.window->f_screen_dimensions().y - f_distance_from_edge}, {44, 9}}, orbs{svc, player, "hud_orb_font", {8.f, 2.f}, {9, 8}},
	  ammo{svc, player, "hud_ammo", {f_distance_from_edge, svc.window->f_screen_dimensions().y - f_distance_from_edge}, {5, 14}} {
	set_position(m_origin);
}

void HUD::update(automa::ServiceProvider& svc, player::Player& player) {
	hearts.update(svc, player);
	orbs.update(svc, player);
	ammo.update(svc, player);
	gun.update(svc, player);
}

void HUD::render(automa::ServiceProvider& svc, player::Player& player, sf::RenderWindow& win) {
	hearts.render(svc, player, win);
	auto offset = hearts.get_offset();
	orbs.render(svc, player, win, offset);
	offset += orbs.get_offset();
	ammo.render(svc, player, win, offset);
	offset += ammo.get_offset();
	gun.render(svc, player, win, offset);
}

} // namespace fornani::gui
