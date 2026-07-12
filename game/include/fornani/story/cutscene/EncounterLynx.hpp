
#pragma once

#include <fornani/story/Cutscene.hpp>

namespace fornani {

class EncounterLynx final : public Cutscene {
  public:
	explicit EncounterLynx(automa::ServiceProvider& svc);
	void update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) override;

  private:
};

} // namespace fornani
