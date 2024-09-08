
#pragma once

#include <SFML/Graphics.hpp>
#include "../utils/Counter.hpp"
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

class Cutscene {
  public:
	Cutscene(int id);
	virtual void update(automa::ServiceProvider& svc, gui::Console& console, world::Map& map, player::Player& player){};

  protected:
	int id{};
};

} // namespace fornani