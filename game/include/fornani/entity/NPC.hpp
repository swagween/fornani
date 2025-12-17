
#pragma once

#include <djson/json.hpp>
#include <fornani/entities/Animation/StateMachine.hpp>
#include <fornani/entities/Mobile.hpp>
#include <fornani/entities/npc/Vendor.hpp>
#include <fornani/entity/Entity.hpp>
#include <fornani/story/Quest.hpp>
#include <fornani/utils/Circuit.hpp>
#include <fornani/utils/ID.hpp>
#include <fornani/utils/StateFunction.hpp>
#include <memory>
#define NPC_BIND(f) [this]() { return this->f(); }

namespace fornani {

enum class NPCFlags { has_turn_animation, face_player, background, no_animation };
enum class NPCState { engaged, force_interact, introduced, talking, cutscene, piggybacking, hidden, distant_interact, just_engaged };
enum class NPCAnimationState { idle, turn, walk, inspect, fall, land };

class NPC : public Entity, public Mobile, public StateMachine<NPCAnimationState> {
  public:
	NPC(automa::ServiceProvider& svc, dj::Json const& in);
	NPC(automa::ServiceProvider& svc, world::Map& map, dj::Json const& in);
	NPC(automa::ServiceProvider& svc, world::Map& map, std::string_view label, bool include_collider = true);
	NPC(automa::ServiceProvider& svc, int id, std::string_view label, std::vector<std::vector<int>> const suites);
	void init(automa::ServiceProvider& svc, dj::Json const& in_data);

	void serialize(dj::Json& out) override;
	void unserialize(dj::Json const& in) override;
	void expose() override;
	void update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] std::optional<std::unique_ptr<gui::Console>>& console, [[maybe_unused]] player::Player& player) override;
	void render(sf::RenderWindow& win, sf::Vector2f cam, float size = 1.f) override;

	/* conversation */
	void start_conversation(automa::ServiceProvider& svc, std::optional<std::unique_ptr<gui::Console>>& console);
	void push_conversation(int convo);
	void pop_conversation();
	void flush_conversations();
	void force_engage();
	void disengage();

	/* animation */
	fsm::StateFunction state_function{[this]() { return this->update_idle(); }};
	fsm::StateFunction update_idle();
	fsm::StateFunction update_turn();
	fsm::StateFunction update_walk();
	fsm::StateFunction update_inspect();
	fsm::StateFunction update_fall();
	fsm::StateFunction update_land();

	/* gameplay helpers */
	void walk();
	void hide() { m_state.set(NPCState::hidden); }
	void unhide() { m_state.reset(NPCState::hidden); }
	void set_position(sf::Vector2f pos);
	void set_position_from_scaled(sf::Vector2f scaled_pos);

	[[nodiscard]] auto is_hidden() const -> bool { return m_state.test(NPCState::hidden); }
	[[nodiscard]] auto is_background() const -> bool { return m_flags.test(NPCFlags::background); }
	[[nodiscard]] auto was_introduced() const -> bool { return m_state.test(NPCState::introduced); }
	[[nodiscard]] auto is_force_interact() const -> bool { return m_state.test(NPCState::force_interact); }
	[[nodiscard]] auto get_number_of_suites() const -> int { return static_cast<int>(conversations.size()); }
	[[nodiscard]] auto get_tag() const -> std::string { return m_label; }
	[[nodiscard]] auto get_specifier() const -> int { return m_id.get(); }
	[[nodiscard]] auto get_vendor_id() const -> int { return get_specifier(); }

	[[nodiscard]] auto get_vendor() const -> std::optional<npc::Vendor*> { return vendor; }

  protected:
	void set_force_interact(bool to) { to ? m_state.set(NPCState::force_interact) : m_state.reset(NPCState::force_interact); }
	void set_distant_interact(bool to) { to ? m_state.set(NPCState::distant_interact) : m_state.reset(NPCState::distant_interact); }

  private:
	bool change_state(NPCAnimationState next, anim::Parameters params);

	/* gameplay members */
	util::BitFlags<NPCState> m_state{};
	util::BitFlags<NPCFlags> m_flags{};
	util::Circuit m_current_conversation;
	std::deque<int> conversations{};
	Animatable m_indicator;
	sf::Vector2f m_offset{};
	std::optional<npc::Vendor*> vendor;
	int current_location{};
	int vendor_id{};
	automa::ServiceProvider* m_services;

	/* data-driven members */
	ID m_id;
	std::string m_label{};
	std::vector<std::vector<int>> m_suites{};
	std::vector<QuestContingency> m_contingencies{};

	bool m_background{};
	bool m_hidden{};
};

} // namespace fornani
