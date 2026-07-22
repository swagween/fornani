#pragma once

#include <fornani/story/Cutscene.hpp>

namespace fornani {

class RetrieveLynx final : public Cutscene {
  public:
	explicit RetrieveLynx(automa::ServiceProvider& svc);
	void update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) override;

  private:
};

} // namespace fornani
