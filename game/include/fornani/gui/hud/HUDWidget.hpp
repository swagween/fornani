
#pragma once

#include <fornani/graphics/Animatable.hpp>
#include <fornani/gui/StatusBar.hpp>
#include <fornani/gui/WidgetBar.hpp>
#include <fornani/utils/Flaggable.hpp>
#include <fornani/utils/Polymorphic.hpp>
#include <optional>
#include <string_view>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::player {
class Player;
}

namespace fornani::gui {

enum class HUDWidgetFlags { hide };

class HUDWidget : public UniquePolymorphic, public Flaggable<HUDWidgetFlags> {
  public:
	HUDWidget(automa::ServiceProvider& svc, player::Player& player, sf::Vector2f root, sf::Vector2f dimensions);
	HUDWidget(automa::ServiceProvider& svc, player::Player& player, std::string_view tag, sf::Vector2f root, sf::Vector2i dimensions);

	virtual void update(automa::ServiceProvider& svc, player::Player& player);
	virtual void render(automa::ServiceProvider& svc, player::Player& player, sf::RenderWindow& win, sf::Vector2f offset = {});
	[[nodiscard]] virtual auto get_offset() const -> sf::Vector2f;

	[[nodiscard]] auto get_root() const -> sf::Vector2f { return p_root; }

  protected:
	std::optional<Animatable> p_animatable{};
	std::optional<sf::FloatRect> p_rect{};
	sf::Vector2f p_root{};
};

} // namespace fornani::gui
