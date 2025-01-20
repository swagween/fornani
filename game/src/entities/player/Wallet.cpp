#include "fornani/entities/player/Wallet.hpp"

namespace player {
void Wallet::set_balance(int balance) { orbs.set(balance); }
void Wallet::add(int amount) { orbs.update(amount); }
void Wallet::subtract(int amount) { orbs.update(-amount); }
} // namespace player
