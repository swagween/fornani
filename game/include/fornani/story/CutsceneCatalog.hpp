
#pragma once

#include "Cutscene.hpp"

#include <memory>
#include <vector>

namespace fornani::automa {
struct ServiceProvider;
}
namespace fornani::gui {
class Console;
}
namespace fornani::world {
class Map;
}
namespace fornani::player {
class Player;
}
namespace fornani {

class CutsceneCatalog {
  public:
	CutsceneCatalog() = default;
	explicit CutsceneCatalog(automa::ServiceProvider& svc);
	void update();
	void push_cutscene(automa::ServiceProvider& svc, world::Map& map, int id);

	std::vector<std::unique_ptr<Cutscene>> cutscenes{};
};

} // namespace fornani
