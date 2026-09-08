#pragma once

#include <fornani/story/Cutscene.hpp>

namespace fornani {

class MomsGrave final : public Cutscene {
  public:
	explicit MomsGrave(automa::ServiceProvider& svc);
	void update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) override;

  private:
};

} // namespace fornani
