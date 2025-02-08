
#pragma once

#include "fornani/utils/Counter.hpp"

#include <string>

namespace fornani::player {

class Wallet {
  public:
	void set_balance(int balance);
	void add(int amount = 1);
	void subtract(int amount = 1);
	[[nodiscard]] auto get_balance() const -> int { return orbs.get_count(); }
	[[nodiscard]] auto get_balance_string() const -> std::string { return std::to_string(get_balance()); }

  private:
	util::Counter orbs{};
};

} // namespace fornani::player
