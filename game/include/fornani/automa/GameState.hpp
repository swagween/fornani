
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/gui/hud/HUD.hpp>
#include <fornani/utils/Polymorphic.hpp>
#include <fornani/world/Map.hpp>

namespace fornani::player {
class Player;
}

namespace fornani::automa {

enum class StateType { menu, game, dojo };
enum class GameStateFlags { playtest, settings_request, controls_request, ready };
enum class MenuSelection { play, options, quit, controls, tutorial, credits, settings, themes };

struct ServiceProvider;

class GameState : public UniquePolymorphic {

  public:
	std::unordered_map<MenuSelection, int> menu_selection_id{{MenuSelection::play, 0},	 {MenuSelection::options, 1}, {MenuSelection::quit, 2},		{MenuSelection::controls, 0},
															 {MenuSelection::themes, 3}, {MenuSelection::credits, 2}, {MenuSelection::settings, 1}, {MenuSelection::tutorial, 4}};

	GameState(ServiceProvider& svc, player::Player& player, std::string_view scene = "", int room_number = 0);

	virtual void tick_update([[maybe_unused]] ServiceProvider& svc, capo::IEngine& engine);
	virtual void frame_update([[maybe_unused]] ServiceProvider& svc) {}
	virtual void render([[maybe_unused]] ServiceProvider& svc, [[maybe_unused]] sf::RenderWindow& win) {}
	virtual void reload(ServiceProvider& svc, int target_state) {};
	virtual void on_exit() {};
	virtual std::optional<std::reference_wrapper<world::Map>> get_map() { return std::nullopt; }

	[[nodiscard]] auto is_ready() const -> bool { return flags.test(GameStateFlags::ready); }
	[[nodiscard]] auto is(StateType test) const -> bool { return m_type == test; }
	[[nodiscard]] auto get_type() const -> StateType { return m_type; }

	bool debug_mode{false};
	util::BitFlags<GameStateFlags> flags{};

	player::Player* player;
	gui::HUD hud;

	float spacing{24.f};
	float top_buffer{80.f};

  protected:
	std::optional<std::unique_ptr<gui::Console>> m_console;
	std::optional<world::Map> m_map;
	StateType m_type{};
	io::Logger m_logger{"GameState"};
};

} // namespace fornani::automa
