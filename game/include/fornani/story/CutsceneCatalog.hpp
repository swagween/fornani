
#pragma once

#include <fornani/io/Logger.hpp>
#include <fornani/story/Cutscene.hpp>
#include <fornani/systems/Register.hpp>
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
	void update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player);
	void push_cutscene(automa::ServiceProvider& svc, world::Map& map, player::Player& player, int id);

	std::vector<std::unique_ptr<Cutscene>> cutscenes{};

  private:
	Register<int> m_register{};
	io::Logger m_logger{"Cutscene"};
};

} // namespace fornani
