
#pragma once

#include <iostream>
#include <optional>
#include <unordered_map>
#include "../../utils/BitFlags.hpp"

namespace automa {
struct ServiceProvider;
}

namespace gui {
class Console;
}

namespace player {

// a small class to handle player interaction with inspectables, NPCs, or anything else that will trigger the console.
// the transponder will respond to player inputs and set the state of the console and player accordingly.

// these are all triggers, so they will be reset each tick
enum class TransponderActions { skip_ahead, next, exit, down, up, select, skip_released };

class Transponder {
  public:
	void update(automa::ServiceProvider& svc, gui::Console& console);
	void track_shipments(gui::Console& console);
	void flush_shipments();
	void end();

	void skip_ahead();
	void enable_skip();
	void next();
	void exit();
	void go_up();
	void go_down();
	void select();

	bool skipped_ahead() const;
	bool skip_released() const;
	bool requested_next() const;
	bool exited() const;
	bool down() const;
	bool up() const;
	bool selected() const;
	
	[[nodiscard]] auto get_item_shipment() const -> int { return shipments.item; }
	[[nodiscard]] auto get_quest_shipment() const -> int { return shipments.quest; }
	[[nodiscard]] auto get_voice_shipment() const -> int { return shipments.voice; }

  private:
	util::BitFlags<TransponderActions> actions{};
	struct {
		int item{};
		int quest{};
		int voice{};
	} shipments{};
};

} // namespace player
