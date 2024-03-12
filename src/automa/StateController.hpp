
#pragma once

#include <string>
#include "../utils/BitFlags.hpp"

namespace automa {

enum class menu_type { file_select, options, settings, credits };
enum class Actions { trigger_submenu, save_loaded, exit_submenu, trigger };

class StateController {

  public:
	inline void refresh(int id) { source_id = id; }
	std::string next_state{};
	int source_id{};
	menu_type submenu{};

	util::BitFlags<Actions> actions{};
};

}
