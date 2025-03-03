
#pragma once
#include <SFML/Graphics.hpp>
#include <string>

namespace fornani::dir {

// the exclusion of "N" in the class name is purposeful; there is no "neutral" direction between L and R.
// the neutral state is used for setting the und state in specific use cases, like weapon direction.
enum class LR : std::uint8_t { left, neutral, right };
enum class UND : std::uint8_t { up, neutral, down };
// intermediate direction, used for special cases like grappling hook
enum class Inter : std::uint8_t { north, south, east, west, northeast, northwest, southeast, southwest };

struct Direction {
	Direction(UND und_preset = UND::neutral, LR lr_preset = LR::neutral) : und(und_preset), lr(lr_preset) {}
	Direction(sf::Vector2<int> preset) : lr{preset.x == 0 ? LR::neutral : preset.x == 1 ? LR::right : LR::left}, und{preset.y == 0 ? dir::UND::neutral : preset.y == 1 ? dir::UND::down : dir::UND::up} {}

	LR lr{LR::neutral};
	UND und{UND::neutral};
	Inter inter{Inter::north};

	[[nodiscard]] auto up() const -> bool { return und == UND::up; }
	[[nodiscard]] auto down() const -> bool { return und == UND::down; }
	[[nodiscard]] auto left() const -> bool { return lr == LR::left; }
	[[nodiscard]] auto right() const -> bool { return lr == LR::right; }
	[[nodiscard]] auto up_or_down() const -> bool { return up() || down(); }
	[[nodiscard]] auto left_or_right() const -> bool { return left() || right(); }

	constexpr void set_intermediate(bool const left, bool const right, bool const up, bool const down) {

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

	void neutralize_und() { und = UND::neutral; }
	void neutralize_lr() { lr = LR::neutral; }
	void flip(bool const horizontal = true, bool const vertical = false) {
		if (horizontal) { lr = lr == LR::left ? LR::right : LR::left; }
		if (vertical) { und = und == UND::up ? UND::down : UND::up; }
	}
	constexpr float as_float() const { return lr == LR::left ? -1.f : (lr == LR::right ? 1.f : 0.f); }
	constexpr float as_float_und() const { return und == UND::up ? -1.f : (und == UND::down ? 1.f : 0.f); }
	sf::Vector2<float> get_vector() const { return sf::Vector2<float>{as_float(), as_float_und()}; }

	std::string print_und() const { return "UND: " + static_cast<std::string>(und == UND::up ? "UP " : (und == UND::neutral ? "NEUTRAL " : "DOWN ")); }
	std::string print_lr() const { return "LR: " + static_cast<std::string>(lr == LR::left ? "LEFT " : (lr == LR::neutral ? "NEUTRAL " : "RIGHT ")); }
	std::string print_intermediate() const {
		switch (inter) {
		default:
		case Inter::north: return "north"; break;
		case Inter::south: return "south"; break;
		case Inter::east: return "east"; break;
		case Inter::west: return "west"; break;
		case Inter::northwest: return "northwest"; break;
		case Inter::northeast: return "northeast"; break;
		case Inter::southwest: return "southwest"; break;
		case Inter::southeast: return "southeast"; break;
		}
		return "null";
	}
};

} // namespace fornani::dir
