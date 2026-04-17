
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/core/Fwd.hpp>
#include <fornani/events/Subscription.hpp>
#include <fornani/graphics/Transition.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/utils/BitFlags.hpp>
#include <fornani/utils/Cooldown.hpp>
#include <fornani/utils/Polymorphic.hpp>
#include <optional>
#include <string_view>

namespace fornani::gui {
class Console;
}

namespace fornani {

enum class CutsceneFlags { complete, started, delete_me };

class Cutscene : public UniquePolymorphic {
  public:
	Cutscene(automa::ServiceProvider& svc, int id, std::string_view label);

	virtual void update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] SceneContext& context, [[maybe_unused]] world::Map& map, [[maybe_unused]] player::Player& player) {};
	virtual void update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] SceneContext& context, [[maybe_unused]] world::Map& map, [[maybe_unused]] player::Player& player, graphics::Transition& transition) {};
	virtual void render(sf::RenderWindow& win, sf::Vector2f cam) {};
	[[nodiscard]] auto complete() const -> bool { return flags.test(CutsceneFlags::complete); }
	[[nodiscard]] auto delete_me() const -> bool { return flags.test(CutsceneFlags::delete_me); }

	void set_progress(int const to);

  protected:
	util::BitFlags<CutsceneFlags> flags{};
	int progress{};
	int total_conversations{};
	int id{};
	struct {
		bool no_player{};
		int target_state_on_end{};
	} metadata{};
	struct {
		util::Cooldown beginning{256};
		util::Cooldown pause{64};
		util::Cooldown long_pause{512};
		util::Cooldown progressor{16};
		util::Cooldown end{256};
	} cooldowns{};

	// debug
	bool debug{};

	io::Logger p_logger{"Cutscene"};

	std::shared_ptr<Slot const> p_slot{std::make_shared<Slot const>()};
};

} // namespace fornani
