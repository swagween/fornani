
#pragma once

#include <array>
#include <cstdio>
#include <list>
#include <memory>
#include <string>
#include "../setup/EnumLookups.hpp"
#include "../utils/BitFlags.hpp"
#include "../entities/animation/Animation.hpp"
#include "../particle/Gravitator.hpp"
#include "WidgetBar.hpp"
#include "../utils/Stopwatch.hpp"

namespace automa {
struct ServiceProvider;
}

namespace player {
class Player;
}

namespace gui {

enum class HUDState { shield };
inline int const num_bits{64};

class HUD {

  public:
	HUD() = default;
	HUD(automa::ServiceProvider& svc, player::Player& player, sf::Vector2<int> pos);
	void update(automa::ServiceProvider& svc, player::Player& player);
	void render(player::Player& player, sf::RenderWindow& win);
	void set_corner_pad(automa::ServiceProvider& svc, bool file_preview = false);

	sf::Vector2<int> position{};
	int total_hp_cells{};
	int filled_hp_cells{};
	int num_orbs{};
	int shield_bar{num_bits};

	std::string_view gun_name{};
	std::string digits{};

	sf::Vector2f corner_pad{};

	util::BitFlags<HUDState> flags{};

  private:
	struct {
		sf::Vector2<float> hp{};
		sf::Vector2<float> orb{};
		sf::Vector2<float> ammo{};
		sf::Vector2<float> gun{};
		sf::Vector2<float> shield{};
	} origins{};

	struct {
		sf::Sprite orb{};
		sf::Sprite gun{};
		sf::Sprite gun_shadow{};
		sf::Sprite pointer{};
		sf::Sprite shield_icon{};
		sf::Sprite shield_bit{};
	} sprites{};

	WidgetBar health_bar{};
	WidgetBar ammo_bar{};

	sf::Vector2<int> heart_dimensions{18, 18};
	sf::Vector2<float> f_heart_dimensions{18.f, 18.f};
	sf::Vector2<int> orb_text_dimensions{18, 16};
	sf::Vector2<int> const ammo_dimensions{10, 28};
	sf::Vector2<float> const f_ammo_dimensions{10.f, 28.f};
	sf::Vector2<int> const gun_dimensions{66, 18};
	sf::Vector2<int> const shield_dimensions{18, 18};
	sf::Vector2<int> const shield_bit_dimensions{6, 18};
	sf::Vector2<int> const pointer_dimensions{14, 10};

	int PAD{4};
	int shield_pad{4};
	int HP_pad{2};
	int AMMO_pad{2};
	int orb_pad{8};
	int gun_pad_horiz{12};
	int gun_pad_vert{4};
	int pointer_pad{4};
	int orb_label_width{44};
	int orb_label_index{10};

	int num_heart_sprites{4};
	int num_orb_chars{11};
	int num_guns{18}; // to be changed later, obviously
	int num_colors{7};

	vfx::Gravitator shield_discrepancy{};
};

} // namespace gui
