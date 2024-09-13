
#include "CutsceneCatalog.hpp"
#include "cutscenes/LadyNimbusIntro.hpp"

namespace fornani {

CutsceneCatalog::CutsceneCatalog(automa::ServiceProvider& svc) {}

void CutsceneCatalog::update() {
	std::erase_if(cutscenes, [this](auto const& c) { return c->complete(); });
}

void CutsceneCatalog::push_cutscene(automa::ServiceProvider& svc, world::Map& map, gui::Console& console, int id) {
	switch (id) {
	case 6001: cutscenes.push_back(std::make_unique<LadyNimbusIntro>(svc)); break;
	default: cutscenes.push_back(std::make_unique<LadyNimbusIntro>(svc)); break;
	}
}

} // namespace fornani