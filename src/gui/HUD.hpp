
#pragma once

#include <cstdio>
#include <list>
#include <memory>
#include <string>
#include "../setup/EnumLookups.hpp"
#include "../setup/LookupTables.hpp"

namespace automa {
struct ServiceProvider;
}

namespace player {
class Player;
}

namespace gui {

enum HP_ELEMS { HP_LIGHT = 0, HP_FILLED = 1, HP_TAKEN = 2, HP_GONE = 3 };

inline int const distance_from_edge{20};
inline int const PAD{4};
inline int const HP_pad{2};
inline int const orb_pad{8};
inline int const gun_pad_horiz{12};
inline int const gun_pad_vert{4};
inline int const pointer_pad{4};
inline const sf::Vector2<int> heart_dimensions{18, 18};
inline const sf::Vector2<int> orb_text_dimensions{18, 16};
inline const sf::Vector2<int> gun_dimensions{66, 18};
inline const sf::Vector2<int> pointer_dimensions{14, 10};
inline const sf::Vector2<int> HP_origin{distance_from_edge, static_cast<int>(cam::screen_dimensions.y) - distance_from_edge - heart_dimensions.y};
inline const sf::Vector2<int> ORB_origin{distance_from_edge, HP_origin.y - PAD - orb_text_dimensions.y};
inline const sf::Vector2<int> GUN_origin{distance_from_edge, ORB_origin.y - PAD - pointer_dimensions.y - pointer_pad * 2};
inline int const orb_label_width{44};
inline int const orb_label_index{10};

int const num_heart_sprites = 4;
int const num_orb_chars = 11;
int const num_guns = 18; // to be changed later, obviously
int const num_colors = 6;

class HUD {

  public:
	HUD() = default;
	HUD(automa::ServiceProvider& svc, player::Player& player, sf::Vector2<int> pos);
	void update(player::Player& player);
	void constrain();
	void render(player::Player& player, sf::RenderWindow& win);
	void set_corner_pad(bool file_preview = false);

	sf::Vector2<int> position{};
	int total_hp_cells{};
	int filled_hp_cells{};
	int max_orbs{};
	int num_orbs{};
	std::string_view gun_name{};
	std::string digits{};

	std::array<sf::Sprite, num_heart_sprites> sp_hearts{};
	std::array<sf::Sprite, num_orb_chars> sp_orb_text{};
	std::array<sf::Sprite, num_guns> sp_guns_shadow{};
	std::array<sf::Sprite, num_guns> sp_guns{};
	std::array<sf::Sprite, num_colors> sp_pointer{};

	sf::Vector2f corner_pad{}; // for rendering file preview
};

} // namespace gui
