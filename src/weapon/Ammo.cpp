#include "Ammo.hpp"
#include "../entities/player/Player.hpp"
#include "../service/ServiceProvider.hpp"
#include "../utils/Math.hpp"

namespace arms {

void Ammo::update() {
	restored.update();
	magazine.update();
}

void Ammo::set_max(int amount) {
	magazine.taken_time = 8;
	if (amount == -1) {
		flags.set(AmmoFlags::infinite);
		magazine.set_max(1);
		refill();
		return;
	}
	magazine.set_max(amount);
}

void Ammo::refill() { magazine.refill(); }

void Ammo::use(int amount) {
	if (!infinite()) { magazine.inflict(amount, true); }
}

} // namespace arms