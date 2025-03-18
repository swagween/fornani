#include "fornani/entities/player/AbilityManager.hpp"
#include "fornani/gui/Console.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::player {

void AbilityManager::give_ability(Abilities ability) { ability_flags.set(ability); }

void AbilityManager::give_ability(int ability) { give_ability(static_cast<Abilities>(ability)); }

void AbilityManager::remove_ability(Abilities ability) { ability_flags.reset(ability); }

void AbilityManager::clear() { ability_flags = {}; }

} // namespace fornani::player
