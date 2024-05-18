
#pragma once
#include <string>

namespace dir {

	// the exclusion of "N" in the class name is purposeful; there is no "neutral" direction between L and R.
	// the neutral state is used for setting the und state in specific use cases, like weapon direction.
enum class LR { left, neutral, right };
enum class UND { up, neutral, down };
// intermediate direction, used for special cases like grappling hook
enum class Inter { north, south, east, west, northeast, northwest, southeast, southwest };

struct Direction {

	LR lr{LR::neutral};
	UND und{UND::neutral};
	Inter inter{Inter::north};

	constexpr void set_intermediate(bool left, bool right, bool up, bool down) {

		// no inputs
		inter = lr == LR::left ? Inter::west : Inter::east;

		if (up) { inter = Inter::north; }
		if (down) { inter = Inter::south; }
		if (left) {
			inter = Inter::west;
			if (up) { inter = Inter::northwest; }
			if (down) { inter = Inter::southwest; }
		}
		if (right) {
			inter = Inter::east;
			if (up) { inter = Inter::northeast; }
			if (down) { inter = Inter::southeast; }
		}
	}

	void neutralize_und() {
		und = UND::neutral;
	}
	void neutralize_lr() {
		lr = LR::neutral;
	}
	constexpr float as_float() const { return lr == LR::left ? -1.f : (lr == LR::right ? 1.f : 0.f); }

	std::string print_und() const { return "UND: " + (std::string)(und == UND::up ? "UP " : (und == UND::neutral ? "NEUTRAL " : "DOWN ")); }
	std::string print_lr() const { return "LR: " + (std::string)(lr == LR::left ? "LEFT " : (lr == LR::neutral ? "NEUTRAL " : "RIGHT ")); }
	std::string print_intermediate() const {
		switch (inter) {
		case Inter::north: return "north"; break;
		case Inter::south: return "south"; break;
		case Inter::east: return "east"; break;
		case Inter::west: return "west"; break;
		case Inter::northwest: return "northwest"; break;
		case Inter::northeast: return "northeast"; break;
		case Inter::southwest: return "southwest"; break;
		case Inter::southeast: return "southeast"; break;
		}
	}

};

} // namespace util
