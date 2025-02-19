
#pragma once

#include "fornani/entities/animation/Animation.hpp"
#include "fornani/utils/StateFunction.hpp"
#include <string_view>
#define NPC_BIND(f) std::bind(&NPCAnimation::f, this)

namespace automa {
struct ServiceProvider;
}

namespace npc {

enum class NPCAnimState { idle, turn, walk, inspect };
enum class NPCCommunication { sprite_flip };

class NPCAnimation {
  public:
	NPCAnimation() = default;
	NPCAnimation(automa::ServiceProvider& svc, int id);
	void update();
	anim::Parameters NPC_idle{};
	anim::Parameters NPC_walk{};
	anim::Parameters NPC_turn{};
	anim::Parameters NPC_inspect{};

	anim::Animation animation{};
	util::BitFlags<NPCAnimState> animation_flags{};
	util::BitFlags<NPCCommunication> communication_flags{};
	fsm::StateFunction state_function = std::bind(&NPCAnimation::update_idle, this);
	fsm::StateFunction update_idle();
	fsm::StateFunction update_turn();
	fsm::StateFunction update_walk();
	fsm::StateFunction update_inspect();
	std::string_view m_label{};
};

} // namespace npc
