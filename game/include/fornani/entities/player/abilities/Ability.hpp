
#pragma once

#include <fornani/entities/player/AbilityManager.hpp>
#include <fornani/entities/player/PlayerAnimation.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/utils/Cooldown.hpp>
#include <fornani/utils/Direction.hpp>
#include <fornani/utils/Polymorphic.hpp>

namespace fornani::shape {
class Collider;
}

namespace fornani::world {
class Map;
}

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::audio {
class Soundboard;
}

namespace fornani::player {
enum class AbilityFlags { failed, active, cancelled };
class PlayerController;
class Ability : public Polymorphic {
  public:
	Ability(automa::ServiceProvider& svc, world::Map& map, shape::Collider& collider, Direction direction = {});
	virtual void update(shape::Collider& collider, PlayerController& controller);

	void fail() { m_flags.set(AbilityFlags::failed); }
	void cancel() { m_flags.set(AbilityFlags::cancelled); }

	[[nodiscard]] auto is(AbilityType test) const -> bool { return m_type == test; }
	[[nodiscard]] auto get_type() const -> AbilityType { return m_type; }
	[[nodiscard]] auto get_direction() const -> Direction { return m_direction; }
	[[nodiscard]] auto is_done() const -> bool { return m_duration.is_complete(); }
	[[nodiscard]] auto is_active() const -> bool { return m_flags.test(AbilityFlags::active); }
	[[nodiscard]] auto get_animation() const -> AnimState { return m_state; }
	[[nodiscard]] auto is_animation_request() const -> bool { return m_animation_trigger.running(); }
	[[nodiscard]] auto failed() const -> bool { return m_flags.test(AbilityFlags::failed); }
	[[nodiscard]] auto cancelled() const -> bool { return m_flags.test(AbilityFlags::cancelled); }

  protected:
	util::Cooldown m_duration;
	util::Cooldown m_animation_trigger{8};
	Direction m_direction;
	AnimState m_state{};
	AbilityType m_type;
	util::BitFlags<AbilityFlags> m_flags{};

	io::Logger m_logger{"Ability"};
};
} // namespace fornani::player
