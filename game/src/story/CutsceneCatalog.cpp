
#include "fornani/story/CutsceneCatalog.hpp"
#include <fornani/story/cutscene/BrynPostMiaag.hpp>
#include <fornani/story/cutscene/HaunchEscape.hpp>
#include <fornani/story/cutscene/HaunchIntro.hpp>
#include <fornani/story/cutscene/LadyNimbusIntro.hpp>
#include <fornani/story/cutscene/LothAtWorm.hpp>
#include <fornani/story/cutscene/MainIntro.hpp>
#include <fornani/story/cutscene/NightsideStation.hpp>
#include <fornani/story/cutscene/NightsideWall.hpp>
#include <fornani/story/cutscene/PioneerBaseDebrief.hpp>
#include <fornani/story/cutscene/ReturnToBase.hpp>

namespace fornani {

CutsceneCatalog::CutsceneCatalog(automa::ServiceProvider& svc) {}

void CutsceneCatalog::update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) {
	std::erase_if(cutscenes, [this](auto const& c) {
		m_register.remove(c->get_id());
		return c->delete_me();
	});
	for (auto& cutscene : cutscenes) { cutscene->update(svc, context, map, player); }
}

void CutsceneCatalog::push_cutscene(automa::ServiceProvider& svc, world::Map& map, player::Player& player, int id) {
	if (m_register.contains(id)) {
		NANI_LOG_INFO(m_logger, "Cutscene {} already exists!", id);
		return;
	}
	switch (id) {
	case 1: cutscenes.push_back(std::make_unique<MainIntro>(svc, map, player)); break;
	case 6001: cutscenes.push_back(std::make_unique<LadyNimbusIntro>(svc)); break;
	case 509: cutscenes.push_back(std::make_unique<BrynPostMiaag>(svc)); break;
	case 300: cutscenes.push_back(std::make_unique<PioneerBaseDebrief>(svc, map, player)); break;
	case 901: cutscenes.push_back(std::make_unique<NightsideStation>(svc)); break;
	case 903: cutscenes.push_back(std::make_unique<NightsideWall>(svc)); break;
	case 268: cutscenes.push_back(std::make_unique<LothAtWorm>(svc)); break;
	case 900: cutscenes.push_back(std::make_unique<HaunchIntro>(svc)); break;
	case 902: cutscenes.push_back(std::make_unique<HaunchEscape>(svc)); break;
	case 209: cutscenes.push_back(std::make_unique<ReturnToBase>(svc)); break;
	default:
		NANI_LOG_INFO(m_logger, "You forgot to add cutscene {} to catalog.", id);
		cutscenes.push_back(std::make_unique<LadyNimbusIntro>(svc));
		break;
	}
	m_register.add(id);
	NANI_LOG_INFO(m_logger, "Pushed Cutscene: {}", id);
}

} // namespace fornani
