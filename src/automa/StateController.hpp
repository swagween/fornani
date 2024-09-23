#pragma once

#include <string>
#include <SFML/Graphics.hpp>
#include "../utils/BitFlags.hpp"

namespace flfx {
class Transition;
}
namespace automa {

enum class menu_type { main, file_select, options, settings, credits, controls, tutorial };
enum class Actions { trigger_submenu, save_loaded, exit_submenu, trigger, shutdown, player_death, intro_done, retry, death_mode, sleep, main_menu, print_stats, screenshot, end_demo, delete_file };
enum class Status { out_of_bounds };

class StateController {

  public:
	inline void refresh(int id) { source_id = id; }
	void switch_rooms(int source, int destination, flfx::Transition& transition);
	int next_state{};
	int demo_level{};
	int source_id{};
	int save_point_id{};
	menu_type submenu{};
	sf::Vector2<float> player_position{};
	util::BitFlags<Actions> actions{};
	util::BitFlags<Status> status{};
};

}
