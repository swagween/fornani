#pragma once

#include <fornani/story/Cutscene.hpp>

namespace fornani {

class FamilyReunion final : public Cutscene {
  public:
	explicit FamilyReunion(automa::ServiceProvider& svc);
	void update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) override;

  private:
};

} // namespace fornani
