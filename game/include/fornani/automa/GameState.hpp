
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/automa/Option.hpp>
#include <fornani/entities/player/Player.hpp>
#include <fornani/graphics/Background.hpp>
#include <fornani/gui/InventoryWindow.hpp>
#include <fornani/gui/PauseWindow.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/gui/hud/HUD.hpp>
#include <fornani/utils/Polymorphic.hpp>
#include <fornani/world/Map.hpp>
#include <filesystem>

namespace fornani::player {
class Player;
}

namespace fornani::automa {

enum class StateType { menu, game };
enum class GameStateFlags { playtest, settings_request, controls_request, ready };
enum class MenuSelection { play, options, quit, controls, tutorial, credits, settings };

struct ServiceProvider;

class GameState : public UniquePolymorphic {

  public:
	std::unordered_map<MenuSelection, int> menu_selection_id{{MenuSelection::play, 0},	   {MenuSelection::options, 1}, {MenuSelection::quit, 2},	 {MenuSelection::controls, 0},
															 {MenuSelection::tutorial, 3}, {MenuSelection::credits, 2}, {MenuSelection::settings, 1}};

	GameState(ServiceProvider& svc, player::Player& player, std::string_view scene = "", int room_number = 0);

	virtual void tick_update([[maybe_unused]] ServiceProvider& svc, capo::IEngine& engine);
	virtual void frame_update([[maybe_unused]] ServiceProvider& svc) {}
	virtual void render([[maybe_unused]] ServiceProvider& svc, [[maybe_unused]] sf::RenderWindow& win) {}
	virtual std::optional<std::reference_wrapper<world::Map>> get_map() { return std::nullopt; }

	[[nodiscard]] auto is_ready() const -> bool { return flags.test(GameStateFlags::ready); }
	[[nodiscard]] auto get_type() const -> StateType { return m_type; }

	bool debug_mode{false};
	util::BitFlags<GameStateFlags> flags{};

	player::Player* player;
	gui::HUD hud;

	float spacing{24.f};
	float top_buffer{80.f};

  protected:
	std::optional<std::unique_ptr<gui::Console>> m_console;
	StateType m_type{};
	io::Logger m_logger{"GameState"};
};

} // namespace fornani::automa
