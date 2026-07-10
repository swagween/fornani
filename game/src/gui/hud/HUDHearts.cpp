
#include <fornani/entities/player/Player.hpp>
#include <fornani/events/GameplayEvent.hpp>
#include <fornani/gui/hud/HUDHearts.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani::gui {

HUDHearts::HUDHearts(automa::ServiceProvider& svc, player::Player& player, sf::Vector2f root, sf::Vector2f dimensions) : HUDWidget{svc, player, root, dimensions}, m_dimensions{dimensions} {
	svc.events.health_increase_event.attach_to(p_slot, &HUDHearts::refresh, this);
	m_health_bar.emplace(svc, static_cast<int>(player.health.get_capacity()), sf::Vector2i{dimensions}, "heads_up_display_hearts", root, -2.f);
}

void HUDHearts::update(automa::ServiceProvider& svc, player::Player& player) {
	HUDWidget::update(svc, player);
	auto& hp = player.health;
	if (m_health_bar) {
		m_health_bar->set_quantity(hp.get_i_capacity());
		m_health_bar->update(svc, hp, hp.has_flag_set(HealthFlags::hit));
		if (p_rect) { m_health_bar->set_origin({0.f, p_rect->size.y}); }
	}
	player.health.set_flag(HealthFlags::hit, false);
}

void HUDHearts::render(automa::ServiceProvider& svc, player::Player& player, sf::RenderWindow& win, sf::Vector2f offset) {
	HUDWidget::render(svc, player, win, offset);
	if (m_health_bar) {
		m_health_bar->set_position(get_root() + offset + sf::Vector2f{-2.f, 0.f});
		m_health_bar->render(win);
	}
}

void HUDHearts::refresh(automa::ServiceProvider& svc, player::Player& player) { m_health_bar.emplace(svc, static_cast<int>(player.health.get_capacity()), sf::Vector2i{m_dimensions}, "heads_up_display_hearts", p_root, -2.f); }

} // namespace fornani::gui
