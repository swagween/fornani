
#pragma once

#include <fornani/gui/StatusBar.hpp>
#include <fornani/gui/WidgetBar.hpp>
#include <optional>
#include <string>
#include "fornani/particle/Gravitator.hpp"
#include "fornani/utils/BitFlags.hpp"
#include "fornani/utils/Stopwatch.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::player {
class Player;
}

namespace fornani::gui {

class HUD {

  public:
	HUD(automa::ServiceProvider& svc, player::Player& player);
	void update(automa::ServiceProvider& svc, player::Player& player);
	void render(player::Player& player, sf::RenderWindow& win);
	void orient(automa::ServiceProvider& svc, player::Player& player, bool file_preview = false);

  private:
	struct {
		sf::Vector2f hp{};
		sf::Vector2f orb{};
		sf::Vector2f ammo{};
		sf::Vector2f gun{};
	} origins{};

	struct {
		sf::Sprite orb;
		sf::Sprite gun;
		sf::Sprite pointer;
	} sprites;

	StatusBar m_reload_bar;
	WidgetBar health_bar;
	std::optional<WidgetBar> ammo_bar{};
	std::string digits{};

	sf::Vector2<int> heart_dimensions{18, 18};
	sf::Vector2f f_heart_dimensions{18.f, 18.f};
	sf::Vector2<int> orb_text_dimensions{18, 16};
	sf::Vector2<int> ammo_dimensions{10, 28};
	sf::Vector2f f_ammo_dimensions{10.f, 28.f};
	sf::Vector2<int> gun_dimensions{66, 18};
	sf::Vector2<int> pointer_dimensions{14, 10};
	sf::Vector2f corner_pad{};

	int PAD{4};
	int HP_pad{2};
	int AMMO_pad{2};
	int orb_pad{8};
	int gun_pad_horiz{12};
	int gun_pad_vert{4};
	int pointer_pad{4};
	int orb_label_width{44};
	int orb_label_index{10};
};

} // namespace fornani::gui
