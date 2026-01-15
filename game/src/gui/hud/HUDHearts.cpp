
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/hud/HUDHearts.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani::gui {

HUDHearts::HUDHearts(automa::ServiceProvider& svc, player::Player& player, sf::Vector2f root, sf::Vector2f dimensions)
	: HUDWidget{svc, player, root, dimensions}, m_health_bar{svc, static_cast<int>(player.health.get_limit()), dimensions, svc.assets.get_texture("hud_hearts"), root} {}

void HUDHearts::update(automa::ServiceProvider& svc, player::Player& player) {
	HUDWidget::update(svc, player);
	auto& hp = player.health;
	m_health_bar.update(svc, hp, hp.flags.test(entity::HPState::hit));
	player.health.flags.reset(entity::HPState::hit);
}

void HUDHearts::render(automa::ServiceProvider& svc, player::Player& player, sf::RenderWindow& win, sf::Vector2f offset) {
	HUDWidget::render(svc, player, win, offset);
	m_health_bar.render(win);
}

} // namespace fornani::gui
