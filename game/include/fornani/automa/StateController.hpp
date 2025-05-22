#pragma once

#include <SFML/Graphics.hpp>
#include "fornani/utils/BitFlags.hpp"

namespace fornani::graphics {
class Transition;
} // namespace fornani::graphics

namespace fornani::automa {

enum class MenuType : std::uint8_t { main, file_select, options, settings, credits, controls, tutorial };
enum class Actions : std::uint8_t { trigger_submenu, save_loaded, exit_submenu, trigger, shutdown, player_death, intro_done, retry, death_mode, main_menu, print_stats, screenshot, end_demo, delete_file };
enum class Status : std::uint8_t { out_of_bounds };

class StateController {

  public:
	void refresh(int const id) { source_id = id; }
	void switch_rooms(int source, int destination, graphics::Transition& transition);
	int next_state{};
	int demo_level{};
	int source_id{};
	int save_point_id{};
	MenuType submenu{};
	sf::Vector2<float> player_position{};
	util::BitFlags<Actions> actions{};
	util::BitFlags<Status> status{};
};

} // namespace fornani::automa
