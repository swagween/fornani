
#pragma once

#include "fornani/entities/packages/Health.hpp"
#include "fornani/utils/BitFlags.hpp"
#include "fornani/utils/Cooldown.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::player {
class Player;
}

namespace fornani::arms {
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
	[[nodiscard]] auto get_native_reload_time() const -> bool { return restored.get_native_time(); }
	[[nodiscard]] auto get_current_reload_time() const -> bool { return restored.get(); }
	[[nodiscard]] auto empty() const -> bool { return magazine.empty(); }
	[[nodiscard]] auto full() const -> bool { return magazine.full() || infinite(); }
	[[nodiscard]] auto infinite() const -> bool { return flags.test(AmmoFlags::infinite); }

	util::Cooldown restored{128};

	entity::Health magazine{};

  private:
	util::BitFlags<AmmoFlags> flags{};
};

} // namespace fornani::arms
