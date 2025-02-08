
#pragma once

#include "fornani/utils/BitFlags.hpp"
#include "fornani/utils/Shipment.hpp"
#include "fornani/utils/QuestCode.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::gui {
class Console;
}

namespace fornani::player {

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

	struct {
		util::Shipment item{};
		util::Shipment quest{};
		util::Shipment voice{};
		util::Shipment emotion{};
		util::Shipment reveal_item{};
	} shipments{};
	util::QuestKey out_quest{};

  private:
	util::BitFlags<TransponderActions> actions{};
};

} // namespace fornani::player
