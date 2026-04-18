
#pragma once

#include <fornani/story/Cutscene.hpp>

namespace fornani {

class PioneerBaseDebrief final : public Cutscene {
  public:
	explicit PioneerBaseDebrief(automa::ServiceProvider& svc, world::Map& map, player::Player& player);
	void update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) override;

  private:
	bool m_ended{};
};

} // namespace fornani
