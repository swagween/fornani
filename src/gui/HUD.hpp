
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
#include "Widget.hpp"
#include "../utils/Stopwatch.hpp"

namespace automa {
struct ServiceProvider;
}

namespace player {
class Player;
}

namespace gui {

inline int const distance_from_edge{20};
inline int const PAD{4};
inline int const shield_pad{4};
inline int const HP_pad{2};
inline int const orb_pad{8};
inline int const gun_pad_horiz{12};
inline int const gun_pad_vert{4};
inline int const pointer_pad{4};
inline const sf::Vector2<int> heart_dimensions{18, 18};
inline const sf::Vector2<int> orb_text_dimensions{18, 16};
inline sf::Vector2<int> const gun_dimensions{66, 18};
inline sf::Vector2<int> const shield_dimensions{18, 18};
inline sf::Vector2<int> const shield_bit_dimensions{6, 18};
inline const sf::Vector2<int> pointer_dimensions{14, 10};
inline int const orb_label_width{44};
inline int const orb_label_index{10};
inline int const num_bits{64};

int const num_heart_sprites{4};
int const num_orb_chars{11};
int const num_guns{18}; // to be changed later, obviously
int const num_colors{7};

enum class HUDState { shield };

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
	int max_orbs{};
	int num_orbs{};
	int shield_bar{num_bits};
	std::string_view gun_name{};
	std::string digits{};

	std::array<sf::Sprite, num_heart_sprites> sp_hearts{};
	std::array<sf::Sprite, num_orb_chars> sp_orb_text{};
	std::array<sf::Sprite, num_guns> sp_guns_shadow{};
	std::array<sf::Sprite, num_guns> sp_guns{};
	std::array<sf::Sprite, num_colors> sp_pointer{};

	sf::Sprite shield_icon{};
	sf::Sprite shield_bit{};

	sf::Vector2f corner_pad{}; // for rendering file preview

	util::BitFlags<HUDState> flags{};

	//debug
	util::Stopwatch stopwatch{};

  private:

	  struct {
		sf::Vector2<float> hp{};
	} origins{};
	  std::vector<Widget> hearts{};

	sf::Vector2<int> HP_origin{};
	sf::Vector2<int> ORB_origin{};
	sf::Vector2<int> GUN_origin{};
	sf::Vector2<int> SHIELD_origin{};

	vfx::Gravitator shield_discrepancy{};

};

} // namespace gui
