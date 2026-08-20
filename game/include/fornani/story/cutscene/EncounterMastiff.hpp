
#pragma once

#include <fornani/story/Cutscene.hpp>

namespace fornani {

class EncounterMastiff final : public Cutscene {
  public:
	explicit EncounterMastiff(automa::ServiceProvider& svc);
	void update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) override;

  private:
};

} // namespace fornani
