
#pragma once

#include <fornani/story/Cutscene.hpp>

namespace fornani {

class BanditEncounter final : public Cutscene {
  public:
	explicit BanditEncounter(automa::ServiceProvider& svc);
	void update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) override;

  private:
};

} // namespace fornani
