
#pragma once

#include <string>
#include "../../utils/Collider.hpp"
#include "../Entity.hpp"
#include "NPCAnimation.hpp"
#include "Vendor.hpp"
#include "../animation/AnimatedSprite.hpp"
#include <deque>
#include <string_view>

namespace automa {
struct ServiceProvider;
}

namespace world {
class Map;
}

namespace gui {
class Console;
}

namespace player {
class Player;
}

namespace npc {

enum class NPCState { engaged, force_interact, introduced, background, talking, cutscene };
enum class NPCTrigger { distant_interact, engaged, cutscene };

class NPC : public entity::Entity {
  public:
	NPC(automa::ServiceProvider& svc, int id);
	void update(automa::ServiceProvider& svc, world::Map& map, gui::Console& console, player::Player& player);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> campos);
	void force_engage();
	void set_position(sf::Vector2<float> pos);
	void set_position_from_scaled(sf::Vector2<float> scaled_pos);
	void set_id(int new_id);
	void start_conversation(automa::ServiceProvider& svc, gui::Console& console);
	void push_conversation(std::string_view convo);
	void pop_conversation();
	void flush_conversations();
	void push_to_background() { state_flags.set(NPCState::background); }
	[[nodiscard]] auto background() const -> bool { return state_flags.test(NPCState::background); }
	[[nodiscard]] auto num_suites() const -> int { return static_cast<int>(conversations.size()); }
	[[nodiscard]] auto get_id() const -> int { return id; }
	[[nodiscard]] auto is_vendor() const -> bool { return static_cast<bool>(vendor); }
	Vendor& get_vendor() { return vendor.value(); }

	std::string_view label{};

  protected:
	util::BitFlags<NPCState> state_flags{};
	util::BitFlags<NPCTrigger> triggers{};
	std::deque<std::string_view> conversations{};
	shape::Collider collider{};
	sf::Sound voice_sound{};
	std::optional<Vendor> vendor{};
  private:
	std::unique_ptr<NPCAnimation> animation_machine{};
	anim::AnimatedSprite indicator;
	int id{};

	struct {
		float walk_threshold{0.5f};
	} physical{};
};

} // namespace entity
