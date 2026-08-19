
#pragma once

#include <fornani/story/Cutscene.hpp>

namespace fornani {

enum class SpencerRevealFlags : std::uint8_t { player_stopped };

class SpencerReveal final : public Cutscene {
  public:
	explicit SpencerReveal(automa::ServiceProvider& svc);
	void update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) override;

  private:
	util::Cooldown m_intro;
	util::Cooldown m_spencer_reveal;
	util::Cooldown m_dial;
	util::BitFlags<SpencerRevealFlags> m_flags{};
};

} // namespace fornani
