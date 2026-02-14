
#pragma once
#include <SFML/Graphics.hpp>
#include <fornani/utils/BitFlags.hpp>
#include <string>

namespace fornani {

enum class LR { left, right };
enum class LNR { left, right, neutral };
enum class UND { up, down, neutral };
enum class UDLR { up, down, left, right };
enum class Inter { north, south, east, west, northeast, northwest, southeast, southwest };
enum class HV { horizontal, vertical };

enum class DirectionFlags { locked };

class Direction;

class SimpleDirection {
  public:
	SimpleDirection() = default;
	SimpleDirection(Direction const to);
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

struct CardinalDirection {
  public:
	CardinalDirection() = default;
	CardinalDirection(int to) : udlr{static_cast<UDLR>(to)} {}
	CardinalDirection(UDLR to) : udlr{to} {}

	void set(UDLR to) { udlr = to; }
	void set(LR to) { udlr = to == LR::left ? UDLR::left : UDLR::right; }
	void set(LNR to) { udlr = to == LNR::left ? UDLR::left : UDLR::right; }
	void set(UND to) { udlr = to == UND::up ? UDLR::up : UDLR::down; }

	[[nodiscard]] auto get() const -> UDLR { return udlr; }
	[[nodiscard]] auto up() const -> bool { return udlr == UDLR::up; }
	[[nodiscard]] auto down() const -> bool { return udlr == UDLR::down; }
	[[nodiscard]] auto left() const -> bool { return udlr == UDLR::left; }
	[[nodiscard]] auto right() const -> bool { return udlr == UDLR::right; }
	[[nodiscard]] auto print() const -> std::string { return left() ? "left" : right() ? "right" : up() ? "up" : "down"; }
	[[nodiscard]] auto as_hv() const -> HV { return up() || down() ? HV::vertical : HV::horizontal; }
	[[nodiscard]] auto as_vector() const -> sf::Vector2f { return up() ? sf::Vector2f{0.f, -1.f} : down() ? sf::Vector2f{0.f, 1.f} : left() ? sf::Vector2f{-1.f, 0.f} : sf::Vector2f{1.f, 0.f}; }
	[[nodiscard]] auto as_degrees() const -> float { return up() ? 0.f : down() ? 180.f : left() ? 270.f : 90.f; }

	bool operator==(CardinalDirection const& other) const { return other.udlr == udlr; }
	bool operator!=(CardinalDirection const& other) const { return other.udlr != udlr; }

	template <typename T>
	T as() const {
		return static_cast<T>(udlr);
	}

  private:
	UDLR udlr{};
};

struct Direction {
	Direction(SimpleDirection dir) : Direction(UND::neutral, dir.as<LNR>()) {}
	Direction(UND und_preset = UND::neutral, LNR lnr_preset = LNR::neutral) : und(und_preset), lnr(lnr_preset) {}
	Direction(sf::Vector2i preset) : lnr{preset.x == 0 ? LNR::neutral : preset.x == 1 ? LNR::right : LNR::left}, und{preset.y == 0 ? UND::neutral : preset.y == 1 ? UND::up : UND::down} {}
	Direction(sf::Vector2i preset, bool world_orientation) : lnr{preset.x == 0 ? LNR::neutral : preset.x == 1 ? LNR::right : LNR::left}, und{preset.y == 0 ? UND::neutral : preset.y == -1 ? UND::up : UND::down} {}

	bool operator==(Direction const& other) const { return other.lnr == lnr && other.und == und; }
	bool operator!=(Direction const& other) const { return other.lnr != lnr || other.und != und; }

	LNR lnr{LNR::neutral};
	UND und{UND::neutral};

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

	[[nodiscard]] Inter get_intermediate(bool const left, bool const right, bool const up, bool const down) const {

		// no inputs
		auto ret = lnr == LNR::left ? Inter::west : Inter::east;

		if (up) { ret = Inter::north; }
		if (down) { ret = Inter::south; }
		if (left) {
			ret = Inter::west;
			if (up) { ret = Inter::northwest; }
			if (down) { ret = Inter::southwest; }
		}
		if (right) {
			ret = Inter::east;
			if (up) { ret = Inter::northeast; }
			if (down) { ret = Inter::southeast; }
		}
		return ret;
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

  private:
	util::BitFlags<DirectionFlags> m_flags{};
};

} // namespace fornani
