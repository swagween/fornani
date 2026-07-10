#pragma once

#include "fornani/story/Cutscene.hpp"

namespace fornani {

class LadyNimbusIntro final : public Cutscene {
  public:
	explicit LadyNimbusIntro(automa::ServiceProvider& svc);
	void update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) override;
};

} // namespace fornani
