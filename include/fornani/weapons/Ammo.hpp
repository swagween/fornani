
#pragma once

#include "../../include/fornani/utils/Cooldown.hpp"
#include "../utils/Counter.hpp"
#include "../../include/fornani/utils/Direction.hpp"
#include "../../include/fornani/utils/BitFlags.hpp"
#include "../../include/fornani/entities/packages/Health.hpp"

namespace automa {
struct ServiceProvider;
}

namespace player {
class Player;
}

namespace arms {
enum class AmmoFlags { infinite };
class Ammo {
  public:
	void update();
	void set_max(int amount);
	void refill();
	void use(int amount = 1);
	[[nodiscard]] auto get_count() const -> int { return static_cast<int>(magazine.get_hp()); }
	[[nodiscard]] auto get_capacity() const -> int { return static_cast<int>(magazine.get_max()); }
	[[nodiscard]] auto get_taken_point() const -> int { return static_cast<int>(magazine.taken_point); }
	[[nodiscard]] auto empty() const -> bool { return magazine.empty(); }
	[[nodiscard]] auto full() const -> bool { return magazine.full() || infinite(); }
	[[nodiscard]] auto infinite() const -> bool { return flags.test(AmmoFlags::infinite); }
	util::Cooldown restored{128};

	entity::Health magazine{};

  private:
	util::BitFlags<AmmoFlags> flags{};
};

} // namespace arms
