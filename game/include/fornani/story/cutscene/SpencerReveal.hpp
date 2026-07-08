
#pragma once

#include <fornani/story/Cutscene.hpp>

namespace fornani {

enum class SpencerRevealFlags { player_stopped };

class SpencerReveal final : public Cutscene {
  public:
	explicit SpencerReveal(automa::ServiceProvider& svc);
	void update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) override;

  private:
	util::Cooldown m_intro;
	util::BitFlags<SpencerRevealFlags> m_flags{};
};

} // namespace fornani
