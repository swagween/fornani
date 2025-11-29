
#pragma once
#include <SFML/Graphics.hpp>
#include <fornani/utils/BitFlags.hpp>
#include <string>

namespace fornani {

enum class LR : std::uint8_t { left, right };
enum class LNR : std::uint8_t { left, right, neutral };
enum class UND : std::uint8_t { up, down, neutral };
enum class Inter : std::uint8_t { north, south, east, west, northeast, northwest, southeast, southwest };
enum class HV : std::uint8_t { horizontal, vertical };

enum class DirectionFlags : std::uint8_t { locked };

class SimpleDirection {
  public:
	SimpleDirection() = default;
	SimpleDirection(LR to) : lr{to} {}

	void set(LR to) { lr = to; }
	void set(LNR to) { lr = to == LNR::left ? LR::left : LR::right; }
	void flip() { lr = left() ? LR::right : LR::left; }

	[[nodiscard]] auto left() const -> bool { return lr == LR::left; }
	[[nodiscard]] auto right() const -> bool { return lr == LR::right; }
	[[nodiscard]] auto print() const -> std::string { return left() ? "left" : "right"; }
	[[nodiscard]] auto as_float() const -> float { return lr == LR::left ? -1.f : 1.f; }

	bool operator==(SimpleDirection const& other) const { return other.lr == lr; }
	bool operator!=(SimpleDirection const& other) const { return other.lr != lr; }

	template <typename T>
	T as() const {
		return static_cast<T>(lr);
	}

  private:
	LR lr{};
};

struct Direction {
	Direction(SimpleDirection dir) : Direction(UND::neutral, dir.as<LNR>()) {}
	Direction(UND und_preset = UND::neutral, LNR lnr_preset = LNR::neutral) : und(und_preset), lnr(lnr_preset) {}
	Direction(sf::Vector2i preset) : lnr{preset.x == 0 ? LNR::neutral : preset.x == 1 ? LNR::right : LNR::left}, und{preset.y == 0 ? UND::neutral : preset.y == 1 ? UND::up : UND::down} {}
	Direction(sf::Vector2i preset, bool world_orientation) : lnr{preset.x == 0 ? LNR::neutral : preset.x == 1 ? LNR::right : LNR::left}, und{preset.y == 0 ? UND::neutral : preset.y == -1 ? UND::up : UND::down} {}

	LNR lnr{LNR::neutral};
	UND und{UND::neutral};
	Inter inter{Inter::north};

	[[nodiscard]] auto up() const -> bool { return und == UND::up; }
	[[nodiscard]] auto down() const -> bool { return und == UND::down; }
	[[nodiscard]] auto left() const -> bool { return lnr == LNR::left; }
	[[nodiscard]] auto right() const -> bool { return lnr == LNR::right; }
	[[nodiscard]] auto up_or_down() const -> bool { return up() || down(); }
	[[nodiscard]] auto left_or_right() const -> bool { return left() || right(); }
	[[nodiscard]] auto is_locked() const -> bool { return m_flags.test(DirectionFlags::locked); }

	void set(LNR to) { lnr = is_locked() ? lnr : to; }
	void set(UND to) { und = is_locked() ? und : to; }
	constexpr void set_from_simple(SimpleDirection from) { lnr = from.left() ? LNR::left : LNR::right; }
	void lock() { m_flags.set(DirectionFlags::locked); }
	void unlock() { m_flags.reset(DirectionFlags::locked); }

	constexpr void set_intermediate(bool const left, bool const right, bool const up, bool const down) {

		// no inputs
		inter = lnr == LNR::left ? Inter::west : Inter::east;

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
	void neutralize_lr() { lnr = LNR::neutral; }
	void flip(bool const horizontal = true, bool const vertical = false) {
		if (horizontal) { lnr = lnr == LNR::left ? LNR::right : LNR::left; }
		if (vertical) { und = und == UND::up ? UND::down : UND::up; }
	}
	constexpr float as_float() const { return lnr == LNR::left ? -1.f : (lnr == LNR::right ? 1.f : 0.f); }
	constexpr float as_float_und() const { return und == UND::up ? -1.f : (und == UND::down ? 1.f : 0.f); }
	sf::Vector2f get_vector() const { return sf::Vector2f{as_float(), as_float_und()}; }

	std::string print() const { return print_und() + ", " + print_lnr(); }
	std::string print_und() const { return und == UND::up ? "up" : (und == UND::neutral ? "neutral" : "down"); }
	std::string print_lnr() const { return lnr == LNR::left ? "left" : (lnr == LNR::neutral ? "neutral" : "right"); }
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

  private:
	util::BitFlags<DirectionFlags> m_flags{};
};

} // namespace fornani
