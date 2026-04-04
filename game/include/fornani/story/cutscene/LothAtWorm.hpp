
#pragma once

#include <fornani/story/Cutscene.hpp>

namespace fornani {

enum class LothAtWormFlags { player_stopped, nani_turned };

class LothAtWorm final : public Cutscene {
  public:
	explicit LothAtWorm(automa::ServiceProvider& svc);
	void update(automa::ServiceProvider& svc, std::optional<std::unique_ptr<gui::Console>>& console, world::Map& map, player::Player& player) override;

  private:
	util::Cooldown m_intro;
	util::BitFlags<LothAtWormFlags> m_flags{};
};

} // namespace fornani
