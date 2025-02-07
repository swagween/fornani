
#pragma once

#include <array>
#include <cstdio>
#include <list>
#include <memory>
#include <string>
#include "fornani/setup/EnumLookups.hpp"
#include "fornani/utils/BitFlags.hpp"
#include "fornani/entities/animation/Animation.hpp"
#include "fornani/particle/Gravitator.hpp"
#include "WidgetBar.hpp"
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
		sf::Vector2<float> hp{};
		sf::Vector2<float> orb{};
		sf::Vector2<float> ammo{};
		sf::Vector2<float> gun{};
	} origins{};

	struct {
		sf::Sprite orb;
		sf::Sprite gun;
		sf::Sprite pointer;
	} sprites;

	WidgetBar health_bar{};
	WidgetBar ammo_bar{};
	std::string digits{};

	sf::Vector2<int> heart_dimensions{18, 18};
	sf::Vector2<float> f_heart_dimensions{18.f, 18.f};
	sf::Vector2<int> orb_text_dimensions{18, 16};
	sf::Vector2<int> ammo_dimensions{10, 28};
	sf::Vector2<float> f_ammo_dimensions{10.f, 28.f};
	sf::Vector2<int> gun_dimensions{66, 18};
	sf::Vector2<int> pointer_dimensions{14, 10};

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

} // namespace gui
