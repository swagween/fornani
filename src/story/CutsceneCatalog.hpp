
#pragma once

#include <SFML/Graphics.hpp>
#include "../utils/Counter.hpp"
#include "Cutscene.hpp"

#include <memory>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace automa {
struct ServiceProvider;
}
namespace gui {
class Console;
}
namespace world {
class Map;
}
namespace player {
class Player;
}
namespace fornani {

class CutsceneCatalog {
  public:
	CutsceneCatalog() = default;
	CutsceneCatalog(automa::ServiceProvider& svc);
	void update();
	void push_cutscene(automa::ServiceProvider& svc, world::Map& map, gui::Console& console, int id);

	std::vector<std::unique_ptr<Cutscene>> cutscenes{};
};

} // namespace fornani