
#pragma once

#include <fornani/core/Fwd.hpp>
#include <fornani/graphics/rewards/IRewardSequence.hpp>
#include <fornani/particle/Effect.hpp>
#include <fornani/particle/Emitter.hpp>
#include <fornani/particle/Sparkler.hpp>
#include <fornani/utils/Cooldown.hpp>
#include <fornani/utils/Flaggable.hpp>

namespace fornani::graphics {

enum class AbilityRewardSequenceFlags : std::uint8_t {};

class AbilityRewardSequence : public IRewardSequence, public Flaggable<AbilityRewardSequenceFlags> {
  public:
	AbilityRewardSequence(automa::ServiceProvider& svc, player::Player& player, SceneContext& context);
	void update(automa::ServiceProvider& svc, player::Player& player, world::Map& map) override;
	void render(sf::RenderWindow& win) override;

  private:
	std::optional<entity::Effect> m_effect{};
	std::optional<std::unique_ptr<vfx::Emitter>> m_emitter{};

	util::Cooldown m_cinematic;
	util::Cooldown m_flash;

	vfx::Sparkler m_sparkler;
};

} // namespace fornani::graphics
