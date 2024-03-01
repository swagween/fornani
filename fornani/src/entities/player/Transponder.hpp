
#pragma once

#include <iostream>
#include <optional>
#include <unordered_map>
#include "../../utils/BitFlags.hpp"

namespace player {

// a small class to handle player interaction with inspectables, NPCs, or anything else that will trigger the console.
// the transponder will respond to player inputs and set the state of the console and player accordingly.

// these are all triggers, so they will be reset each tick
enum class TransponderActions { skip_ahead, next, exit };

class Transponder {
  public:
	void update();
	void end();

	void skip_ahead();
	void next();
	void exit();

	bool skipped_ahead() const;
	bool requested_next() const;
	bool exited() const;

  private:
	util::BitFlags<TransponderActions> actions{};
};

} // namespace player
