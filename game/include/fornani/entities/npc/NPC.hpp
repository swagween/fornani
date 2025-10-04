
#pragma once

#include <fornani/entities/Animation/StateMachine.hpp>
#include <fornani/entities/Mobile.hpp>
#include <fornani/utils/Circuit.hpp>
#include <fornani/utils/ID.hpp>
#include "fornani/entities/Entity.hpp"
#include "fornani/entities/animation/AnimatedSprite.hpp"
#include "fornani/entities/npc/NPCAnimation.hpp"
#include "fornani/entities/npc/Vendor.hpp"
#include "fornani/io/Logger.hpp"
#include "fornani/utils/Collider.hpp"

#include <deque>
#include <optional>
#include <string_view>

#define NPC_BIND(f) std::bind(&NPC::f, this)

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::world {
class Map;
}

namespace fornani::gui {
class Console;
}

namespace fornani::player {
class Player;
}

namespace fornani::npc {

enum class NPCFlags : std::uint8_t { has_turn_animation };
enum class NPCState : std::uint8_t { engaged, force_interact, introduced, background, talking, cutscene, piggybacking, hidden };
enum class NPCTrigger : std::uint8_t { distant_interact, engaged, cutscene };
enum class NPCAnimationState : std::uint8_t { idle, turn, walk, inspect, fall, land };

class NPC : public Mobile, public StateMachine<NPCAnimationState> {
  public:
	NPC(automa::ServiceProvider& svc, std::string_view label);
	void update(automa::ServiceProvider& svc, world::Map& map, std::optional<std::unique_ptr<gui::Console>>& console, player::Player& player);
	void post_update(automa::ServiceProvider& svc, world::Map& map, player::Player& player) override;
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2f campos);
	void force_engage();
	void disengage();
	void set_position(sf::Vector2f pos);
	void apply_force(sf::Vector2f force) { collider.physics.apply_force(force); };
	void set_position_from_scaled(sf::Vector2f scaled_pos);
	void start_conversation(automa::ServiceProvider& svc, std::optional<std::unique_ptr<gui::Console>>& console);
	void push_conversation(int convo);
	void pop_conversation();
	void flush_conversations();
	void push_to_background() { state_flags.set(NPCState::background); }
	void hide() { state_flags.set(NPCState::hidden); }
	void unhide() { state_flags.reset(NPCState::hidden); }
	void set_current_location(int location) { current_location = location; }
	void walk();

	// animation
	fsm::StateFunction state_function = std::bind(&NPC::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_turn();
	fsm::StateFunction update_walk();
	fsm::StateFunction update_inspect();
	fsm::StateFunction update_fall();
	fsm::StateFunction update_land();

	[[nodiscard]] auto background() const -> bool { return state_flags.test(NPCState::background); }
	[[nodiscard]] auto num_suites() const -> int { return static_cast<int>(conversations.size()); }
	[[nodiscard]] auto get_id() const -> int { return m_id.get_specifier(); }
	[[nodiscard]] auto is_vendor() const -> bool { return static_cast<bool>(vendor); }
	[[nodiscard]] auto piggybacking() const -> bool { return state_flags.test(NPCState::piggybacking); }
	[[nodiscard]] auto hidden() const -> bool { return state_flags.test(NPCState::hidden); }
	[[nodiscard]] auto get_label() const -> std::string { return m_label; }

	Vendor& get_vendor() const { return *vendor.value(); }

  protected:
	util::BitFlags<NPCState> state_flags{};
	util::BitFlags<NPCTrigger> triggers{};
	util::BitFlags<NPCFlags> p_flags{};
	std::deque<int> conversations{};
	std::optional<Vendor*> vendor;
	std::string m_label{};
	sf::Vector2f m_offset{};

	io::Logger m_logger{"npc"};

  private:
	bool change_state(NPCAnimationState next, anim::Parameters params);
	Animatable m_indicator;
	int current_location{};
	util::Circuit m_current_conversation;
	ID m_id;

	struct {
		float walk_threshold{0.5f};
	} physical{};
};

} // namespace fornani::npc
