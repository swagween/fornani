
#pragma once
#include "fornani/story/Cutscene.hpp"

namespace fornani {

class LadyNimbusIntro : public Cutscene {
  public:
	LadyNimbusIntro(automa::ServiceProvider& svc);
	void update(automa::ServiceProvider& svc, gui::Console& console, world::Map& map, player::Player& player);
  private:
};

} // namespace fornani