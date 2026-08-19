
#pragma once

#include <fornani/story/Cutscene.hpp>
#include <fornani/utils/Flaggable.hpp>

namespace fornani {

enum class MainIntroFlags : std::uint8_t { player_stopped, rumble, takeover, start_takeover };

class MainIntro final : public Cutscene, public Flaggable<MainIntroFlags> {
  public:
	explicit MainIntro(automa::ServiceProvider& svc, world::Map& map, player::Player& player);
	void update(automa::ServiceProvider& svc, SceneContext& context, world::Map& map, player::Player& player) override;

  private:
	util::Cooldown m_outro;
	util::Cooldown m_willett_walk;
};

} // namespace fornani
