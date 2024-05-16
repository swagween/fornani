
#pragma once

#include "../utils/Cooldown.hpp"
#include "../utils/Direction.hpp"

namespace automa {
struct ServiceProvider;
}

namespace player {
class Player;
}

namespace arms {
	class Ammo {
		public:
	  private:
		  struct {
			int amount{};
		  } variables{};
	};
} // namespace arms
