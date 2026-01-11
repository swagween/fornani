
#include "fornani/story/CutsceneCatalog.hpp"
#include <fornani/story/cutscene/BrynPostMiaag.hpp>
#include <fornani/story/cutscene/LadyNimbusIntro.hpp>
#include <fornani/story/cutscene/MainIntro.hpp>
#include <fornani/story/cutscene/PioneerBaseDebrief.hpp>

namespace fornani {

CutsceneCatalog::CutsceneCatalog(automa::ServiceProvider& svc) {}

void CutsceneCatalog::update() {
	std::erase_if(cutscenes, [this](auto const& c) { return c->complete(); });
}

void CutsceneCatalog::push_cutscene(automa::ServiceProvider& svc, world::Map& map, player::Player& player, int id) {
	switch (id) {
	case 1: cutscenes.push_back(std::make_unique<MainIntro>(svc, map, player)); break;
	case 6001: cutscenes.push_back(std::make_unique<LadyNimbusIntro>(svc)); break;
	case 509: cutscenes.push_back(std::make_unique<BrynPostMiaag>(svc)); break;
	case 300: cutscenes.push_back(std::make_unique<PioneerBaseDebrief>(svc, map, player)); break;
	default: cutscenes.push_back(std::make_unique<LadyNimbusIntro>(svc)); break;
	}
}

} // namespace fornani
