
#pragma once

#include <fornani/story/Cutscene.hpp>

namespace fornani {

class BanditConfrontation final : public Cutscene {
  public:
	explicit BanditConfrontation(automa::ServiceProvider& svc);
	void update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) override;

  private:
	util::Cooldown m_intro;
	util::Cooldown m_bandit_escape;
	util::Cooldown m_threaten;
};

} // namespace fornani
