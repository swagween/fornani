
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/hud/HUDWidget.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani::gui {

HUDWidget::HUDWidget(automa::ServiceProvider& svc, player::Player& player, sf::Vector2f root, sf::Vector2f dimensions) : p_rect{sf::FloatRect{root, dimensions}}, p_root{root} {}

HUDWidget::HUDWidget(automa::ServiceProvider& svc, player::Player& player, std::string_view tag, sf::Vector2f root, sf::Vector2i dimensions) : p_animatable{Animatable{svc, tag, dimensions}}, p_root{root} {
	p_animatable->set_origin(sf::Vector2f{0.f, p_animatable->get_f_dimensions().y});
}

void HUDWidget::update(automa::ServiceProvider& svc, player::Player& player) {}

void HUDWidget::render(automa::ServiceProvider& svc, player::Player& player, sf::RenderWindow& win, sf::Vector2f offset) {}

auto HUDWidget::get_offset(bool scaled) const -> sf::Vector2f {
	auto factor = scaled ? constants::f_scale_factor : 1.f;
	return p_animatable ? sf::Vector2f{0.f, p_animatable->get_f_dimensions().y * factor} : p_rect ? sf::Vector2f{0.f, p_rect->size.y * factor} : sf::Vector2f{};
}

} // namespace fornani::gui
