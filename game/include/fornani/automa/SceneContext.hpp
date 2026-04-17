
#pragma once

#include <fornani/graphics/Transition.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/story/CutsceneCatalog.hpp>

namespace fornani {

struct SceneContext {
	SceneContext(automa::ServiceProvider& svc);
	std::optional<std::unique_ptr<gui::Console>> console;
	graphics::Transition transition;
	CutsceneCatalog cutscene_catalog;
};

} // namespace fornani
