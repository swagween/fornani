
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/hud/HUDWidget.hpp>
#include <fornani/service/ServiceProvider.hpp>

namespace fornani::gui {

HUDWidget::HUDWidget(automa::ServiceProvider& svc, player::Player& player, sf::Vector2f root, sf::Vector2f dimensions) : p_rect{sf::FloatRect{root, dimensions}} {}

HUDWidget::HUDWidget(automa::ServiceProvider& svc, player::Player& player, std::string_view tag, sf::Vector2f root, sf::Vector2i dimensions) : p_animatable{Animatable{svc, tag, dimensions}} {}

void HUDWidget::update(automa::ServiceProvider& svc, player::Player& player) {}

void HUDWidget::render(automa::ServiceProvider& svc, player::Player& player, sf::RenderWindow& win, sf::Vector2f offset) {}

auto HUDWidget::get_offset() const -> sf::Vector2f { return p_animatable ? sf::Vector2f{0.f, p_animatable->get_f_dimensions().y} : p_rect ? sf::Vector2f{0.f, p_rect->size.y} : sf::Vector2f{}; }

} // namespace fornani::gui
