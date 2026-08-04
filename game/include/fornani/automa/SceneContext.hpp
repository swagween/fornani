
#pragma once

#include <fornani/graphics/Colors.hpp>
#include <fornani/graphics/Transition.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/story/CutsceneCatalog.hpp>

namespace fornani {

struct SceneContext {
	SceneContext(automa::ServiceProvider& svc);
	std::optional<std::unique_ptr<gui::Console>> console;
	graphics::Transition transition;
	CutsceneCatalog cutscene_catalog;
	std::optional<std::string> biome{};
	[[nodiscard]] auto get_black(automa::ServiceProvider& svc) const -> sf::Color {
		auto black = colors::ui_black;
		if (auto& themed_black = biome) { black = Color{svc.data.biomes["properties"][*themed_black]["black"]}; }
		return black;
	}
};

} // namespace fornani
