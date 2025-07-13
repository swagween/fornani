
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

namespace fornani::player {
enum class AbilityFlags { failed, active };
class PlayerController;
class Ability : public Polymorphic {
  public:
	Ability(automa::ServiceProvider& svc, world::Map& map, shape::Collider& collider, Direction direction = {});
	virtual void update(shape::Collider& collider, PlayerController& controller);

	[[nodiscard]] auto is(AbilityType test) const -> bool { return m_type == test; }
	[[nodiscard]] auto is_done() const -> bool { return m_duration.is_complete(); }
	[[nodiscard]] auto is_active() const -> bool { return m_flags.test(AbilityFlags::active); }
	[[nodiscard]] auto get_animation() const -> AnimState { return m_state; }
	[[nodiscard]] auto failed() const -> bool { return m_flags.test(AbilityFlags::failed); }

  protected:
	util::Cooldown m_duration;
	Direction m_direction;
	AnimState m_state{};
	AbilityType m_type;
	util::BitFlags<AbilityFlags> m_flags{};

	io::Logger m_logger{"Ability"};
};
} // namespace fornani::player
