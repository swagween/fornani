
#include "fornani/story/CutsceneCatalog.hpp"
#include "fornani/story/cutscene/LadyNimbusIntro.hpp"

namespace fornani {

CutsceneCatalog::CutsceneCatalog(automa::ServiceProvider& svc) {}

void CutsceneCatalog::update() {
	std::erase_if(cutscenes, [this](auto const& c) { return c->complete(); });
}

void CutsceneCatalog::push_cutscene(automa::ServiceProvider& svc, world::Map& map, int id) {
	switch (id) {
	case 6001: cutscenes.push_back(std::make_unique<LadyNimbusIntro>(svc)); break;
	default: cutscenes.push_back(std::make_unique<LadyNimbusIntro>(svc)); break;
	}
}

} // namespace fornani
