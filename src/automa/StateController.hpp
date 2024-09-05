#pragma once

#include <string>
#include <SFML/Graphics.hpp>
#include "../utils/BitFlags.hpp"

namespace automa {

enum class menu_type { main, file_select, options, settings, credits, controls, tutorial };
enum class Actions { trigger_submenu, save_loaded, exit_submenu, trigger, shutdown, player_death, intro_done, retry, death_mode, console_transition, main_menu, print_stats, screenshot, end_demo, delete_file };

class StateController {

  public:
	inline void refresh(int id) { source_id = id; }
	int next_state{};
	int demo_level{};
	int source_id{};
	int save_point_id{};
	menu_type submenu{};
	sf::Vector2<float> player_position{};
	util::BitFlags<Actions> actions{};
};

}
