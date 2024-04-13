#include "AbilityManager.hpp"
#include "../../service/ServiceProvider.hpp"
#include "../../gui/Console.hpp"

namespace player {

void AbilityManager::update(automa::ServiceProvider& svc) {}

void AbilityManager::give_ability(Abilities ability) { ability_flags.set(ability); }

void AbilityManager::give_ability(std::string_view ability) { give_ability(ability_from_label.at(ability)); }

void AbilityManager::remove_ability(Abilities ability) { ability_flags.reset(ability); }

void AbilityManager::clear() { ability_flags = {}; }

} // namespace player
