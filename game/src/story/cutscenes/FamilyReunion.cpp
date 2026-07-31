
#include <fornani/automa/SceneContext.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/story/cutscene/FamilyReunion.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

FamilyReunion::FamilyReunion(automa::ServiceProvider& svc) : Cutscene(svc, 300, "family_reunion") { cooldowns.beginning.start(); }

void FamilyReunion::update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) {}

} // namespace fornani
