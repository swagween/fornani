
#pragma once
#include "../Cutscene.hpp"

namespace fornani {

class LadyNimbusIntro : public Cutscene {
  public:
	void update(automa::ServiceProvider& svc, gui::Console& console, world::Map& map, player::Player& player);
  private:
};

} // namespace fornani