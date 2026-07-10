
#pragma once

#include <djson/json.hpp>
#include <fornani/core/Fwd.hpp>
#include <fornani/entities/Animation/StateMachine.hpp>
#include <fornani/entities/Mobile.hpp>
#include <fornani/entities/npc/Vendor.hpp>
#include <fornani/entity/Entity.hpp>
#include <fornani/events/Subscription.hpp>
#include <fornani/story/Quest.hpp>
#include <fornani/utils/Circuit.hpp>
#include <fornani/utils/Cooldown.hpp>
#include <fornani/utils/Flaggable.hpp>
#include <fornani/utils/ID.hpp>
#include <fornani/utils/StateFunction.hpp>
#include <fornani/utils/TransparentStringHash.hpp>
#include <memory>
#define NPC_BIND(f) [this]() { return this->f(); }

namespace fornani {

enum class NPCFlags { has_turn_animation, face_player, background, no_animation, random_walk, cutscene, piggyback, busy, airborne, custom_camera };
enum class NPCState { engaged, force_interact, introduced, talking, cutscene, piggybacking, hidden, distant_interact, just_engaged, random_walk, invisible, interacting };
enum class NPCAnimationState { idle, turn, walk, inspect, fall, land, busy, stagger, respond, special_1, special_2, special_3 };

struct NPCSpawn {
	NPCSpawn(dj::Json const& in) {
		chance = in["chance"].as<float>();
		interval = in["interval"].as<int>();
	}
	float chance;
	int interval;
};

struct NPCVoiceCue {
	std::string tag{};
};

struct NPCSchedule {
	NPCSchedule(dj::Json const& in);
	[[nodiscard]] auto is_here(int room_id, TimeOfDay tod) const -> bool { return destinations.contains(tod) ? destinations.at(tod) == room_id : true; }
	[[nodiscard]] auto get_location(TimeOfDay tod) const -> int { return destinations.contains(tod) ? destinations.at(tod) : 0; }
	std::unordered_map<TimeOfDay, int> destinations{};
};

class NPC : public Entity, public Mobile, public StateMachine<NPCAnimationState>, public Flaggable<NPCFlags> {
  public:
	NPC(automa::ServiceProvider& svc, dj::Json const& in);
	NPC(automa::ServiceProvider& svc, world::Map& map, dj::Json const& in);
	NPC(automa::ServiceProvider& svc, world::Map& map, std::string_view label, bool include_collider = true);
	NPC(automa::ServiceProvider& svc, int id, std::string_view label, std::vector<std::vector<int>> const suites);
	void init(automa::ServiceProvider& svc, dj::Json const& in_data);
	void handle_spawning(automa::ServiceProvider& svc, dj::Json const& in_data);

	void serialize(dj::Json& out) override;
	void unserialize(dj::Json const& in) override;
	void expose() override;
	void update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] SceneContext& context, [[maybe_unused]] player::Player& player) override;
	void render(sf::RenderWindow& win, sf::Vector2f cam, float size = 1.f) override;

	/* conversation */
	void start_conversation(automa::ServiceProvider& svc, std::optional<std::unique_ptr<gui::Console>>& console);
	void push_conversation(int convo);
	void pop_conversation();
	void play_voice_cue(automa::ServiceProvider& svc, int which) const;
	void piggyback_me(automa::ServiceProvider& svc, int id);
	void flush_conversations();
	void flush_and_push(int convo);
	void force_engage();
	void disengage();
	void set_invisible(bool to = true) { to ? m_state.set(NPCState::invisible) : m_state.reset(NPCState::invisible); }
	void set_special_animation(int which);
	void set_busy();
	void start_busy_timer() { m_busy_timer.start(); }
	void use_portal(world::Map& map);

	/* animation */
	fsm::StateFunction state_function{[this]() { return this->update_idle(); }};
	fsm::StateFunction update_idle();
	fsm::StateFunction update_turn();
	fsm::StateFunction update_walk();
	fsm::StateFunction update_inspect();
	fsm::StateFunction update_fall();
	fsm::StateFunction update_land();
	fsm::StateFunction update_busy();
	fsm::StateFunction update_stagger();
	fsm::StateFunction update_respond();
	fsm::StateFunction update_special_1();
	fsm::StateFunction update_special_2();
	fsm::StateFunction update_special_3();

	/* gameplay helpers */
	void walk();
	void hide() { m_state.set(NPCState::hidden); }
	void unhide() { m_state.reset(NPCState::hidden); }
	void set_position(sf::Vector2f pos);
	void set_position_from_scaled(sf::Vector2f scaled_pos);

	[[nodiscard]] auto is_hidden() const -> bool { return m_state.test(NPCState::hidden); }
	[[nodiscard]] auto is_background() const -> bool { return has_flag_set(NPCFlags::background); }
	[[nodiscard]] auto was_introduced() const -> bool { return m_state.test(NPCState::introduced); }
	[[nodiscard]] auto is_force_interact() const -> bool { return m_state.test(NPCState::force_interact); }
	[[nodiscard]] auto get_number_of_suites() const -> int { return static_cast<int>(conversations.size()); }
	[[nodiscard]] auto get_tag() const -> std::string { return m_label; }
	[[nodiscard]] auto get_specifier() const -> int { return m_id.get(); }
	[[nodiscard]] auto get_vendor_id() const -> int { return get_specifier(); }
	[[nodiscard]] auto is_animation_complete() -> bool { return Mobile::animation.is_complete(); }

	[[nodiscard]] auto get_vendor() const -> std::optional<npc::Vendor*> { return vendor; }

  protected:
	void set_force_interact(bool to) { to ? m_state.set(NPCState::force_interact) : m_state.reset(NPCState::force_interact); }
	void set_distant_interact(bool to) { to ? m_state.set(NPCState::distant_interact) : m_state.reset(NPCState::distant_interact); }

	std::shared_ptr<Slot const> slot{std::make_shared<Slot const>()};

  private:
	bool change_state(NPCAnimationState next, std::string_view to);

	std::unordered_map<int, NPCVoiceCue> m_voice_cues{};

	/* gameplay members */
	util::BitFlags<NPCState> m_state{};
	util::Circuit m_current_conversation;
	util::Cooldown m_busy_timer{};
	util::Cooldown m_disappear;
	std::deque<int> conversations{};
	Animatable m_indicator;
	sf::Vector2f m_offset{};
	std::optional<npc::Vendor*> vendor;
	int m_current_location{};
	int vendor_id{};
	int m_walk_chance{};
	automa::ServiceProvider* m_services;

	float m_walk_speed;

	std::optional<NPCSpawn> m_spawn{};
	std::optional<NPCSchedule> m_schedule{};

	/* data-driven members */
	ID m_id;
	std::string m_label{};
	std::vector<std::vector<int>> m_suites{};

	bool m_background{};
	bool m_hidden{};
	bool m_start_busy{};
};

} // namespace fornani
