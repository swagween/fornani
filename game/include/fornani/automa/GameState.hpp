
#pragma once

#include "Option.hpp"
#include "fornani/components/PhysicsComponent.hpp"
#include "fornani/entities/player/Player.hpp"
#include "fornani/graphics/Background.hpp"
#include "fornani/gui/Console.hpp"
#include "fornani/gui/HUD.hpp"
#include "fornani/gui/InventoryWindow.hpp"
#include "fornani/gui/PauseWindow.hpp"
#include "fornani/utils/Polymorphic.hpp"
#include "fornani/world/Map.hpp"

#include <SFML/Graphics.hpp>

#include <filesystem>

namespace fornani::player {
class Player;
}

namespace fornani::automa {

enum class GameStateFlags : std::uint8_t { playtest, settings_request, controls_request, ready };

enum class MenuSelection : std::uint8_t { play, options, quit, controls, tutorial, credits, settings };

constexpr inline float dot_force{0.9f};
constexpr inline float dot_fric{0.86f};
constexpr inline float dot_speed{200.f};
constexpr inline float title_letter_spacing{1.0f};

struct ServiceProvider;

struct Scene {
	struct {
		std::filesystem::path scene{};
		std::filesystem::path region{};
		std::filesystem::path room{};
		std::filesystem::path next_region{};
		std::filesystem::path next_room{};
	} paths{};
	std::string_view label{};
};

class GameState : public UniquePolymorphic {

  public:
	std::unordered_map<MenuSelection, int> menu_selection_id{{MenuSelection::play, 0},	   {MenuSelection::options, 1}, {MenuSelection::quit, 2},	 {MenuSelection::controls, 0},
															 {MenuSelection::tutorial, 2}, {MenuSelection::credits, 3}, {MenuSelection::settings, 1}};

	GameState(ServiceProvider& svc, player::Player& player, std::string_view scene = "", int room_number = 0);

	virtual void tick_update([[maybe_unused]] ServiceProvider& svc, capo::IEngine& engine);
	virtual void frame_update([[maybe_unused]] ServiceProvider& svc) {}
	virtual void render([[maybe_unused]] ServiceProvider& svc, [[maybe_unused]] sf::RenderWindow& win) {}

	[[nodiscard]] auto is_ready() const -> bool { return flags.test(GameStateFlags::ready); }

	bool debug_mode{false};
	util::BitFlags<GameStateFlags> flags{};

	std::string_view scene{};

	vfx::Gravitator left_dot{};
	vfx::Gravitator right_dot{};
	sf::Vector2f dot_pad{24.f, 8.f};

	player::Player* player;
	gui::HUD hud;

	Scene target_folder{};
	std::vector<Option> options{};
	util::Circuit current_selection{1};
	float spacing{24.f};
	float top_buffer{80.f};

  protected:
	std::optional<std::unique_ptr<gui::Console>> m_console;
	io::Logger m_logger{"GameState"};
};

} // namespace fornani::automa
