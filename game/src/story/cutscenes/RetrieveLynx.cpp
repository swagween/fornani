
#include <fornani/automa/SceneContext.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/story/cutscene/RetrieveLynx.hpp>
#include <fornani/world/Map.hpp>

namespace fornani {

RetrieveLynx::RetrieveLynx(automa::ServiceProvider& svc) : Cutscene(svc, 300, "bandit_encounter") { cooldowns.beginning.start(); }

void RetrieveLynx::update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) {}

} // namespace fornani
