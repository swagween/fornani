
#pragma once

#include <fornani/core/Fwd.hpp>
#include <fornani/graphics/Animatable.hpp>
#include <fornani/graphics/rewards/IRewardSequence.hpp>
#include <fornani/particle/Effect.hpp>
#include <fornani/particle/Emitter.hpp>
#include <fornani/particle/Sparkler.hpp>
#include <fornani/utils/Flaggable.hpp>
#include <fornani/utils/RectPath.hpp>

namespace fornani::graphics {

enum class HealthRewardSequenceFlags { slotted, cinematic, show_heart, finished };

class HealthRewardSequence : public IRewardSequence, public Animatable, public Flaggable<HealthRewardSequenceFlags> {
  public:
	HealthRewardSequence(automa::ServiceProvider& svc, player::Player& player, world::Map& map, sf::Vector2f hud_point);
	void update(automa::ServiceProvider& svc, player::Player& player, world::Map& map) override;
	void render(sf::RenderWindow& win) override;

  private:
	int m_status;
	Animatable m_case;
	Animatable m_ui_heart;
	std::optional<entity::Effect> m_effect{};
	std::optional<std::unique_ptr<vfx::Emitter>> m_emitter{};

	util::RectPath m_case_path;
	util::RectPath m_shard_path;
	util::Cooldown m_cinematic;
	util::Cooldown m_heart_path;

	sf::Vector2f m_render_point;
	sf::Vector2f m_hud_point;
	vfx::Sparkler m_sparkler;
};

} // namespace fornani::graphics
