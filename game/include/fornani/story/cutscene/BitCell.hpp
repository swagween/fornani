
#pragma once

#include <fornani/story/Cutscene.hpp>

namespace fornani {

class BitCell final : public Cutscene {
  public:
	explicit BitCell(automa::ServiceProvider& svc);
	void update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) override;

  private:
};

} // namespace fornani
