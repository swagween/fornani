#include "fornani/weapon/Ammo.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/service/ServiceProvider.hpp"
#include "fornani/utils/Math.hpp"

namespace fornani::arms {

void Ammo::update() {
	restored.update();
	magazine.update();
}

void Ammo::set_max(int amount) {
	if (amount == -1) {
		flags.set(AmmoFlags::infinite);
		magazine.set_capacity(1.f);
		refill();
		return;
	}
	magazine.set_capacity(static_cast<float>(amount));
}

void Ammo::refill() { magazine.refill(); }

void Ammo::use(int amount) {
	if (!infinite()) { magazine.inflict(static_cast<float>(amount), true); }
}

} // namespace fornani::arms
