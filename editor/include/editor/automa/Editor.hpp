
#pragma once

#include <SFML/Graphics.hpp>
#include "editor/canvas/Canvas.hpp"
#include "editor/util/BitFlags.hpp"
#include "editor/tool/Tool.hpp"
#include "editor/setup/WindowManager.hpp"
#include "editor/gui/Console.hpp"
#include "editor/automa/PopupHandler.hpp"
#include <imgui-SFML.h>
#include <sstream>
#include <windows.h>
#include <string_view>
#include <filesystem>
#include <cstdio>
#include <memory>
#include <chrono>
#include <imgui.h>

namespace data {
class ResourceFinder;
}

namespace pi {

enum class GlobalFlags { shutdown, palette_mode };
enum class PressedKeys { control, shift, mouse_left, mouse_right, space };

inline char const* styles[static_cast<size_t>(Style::END)];
inline char const* bgs[static_cast<size_t>(Backdrop::END)];
inline std::string_view const& style_list{};
inline std::string styles_str{};

class Editor {
  public:
	int const TILE_WIDTH{32};
	int const NUM_TOOLS{6};
	Editor(char** argv, WindowManager& window, data::ResourceFinder& finder);
	void run();
	void init(std::string const& load_path);
	void handle_events(std::optional<sf::Event> const event, sf::RenderWindow& win);
	void logic();
	void load();
	bool save();
	void render(sf::RenderWindow& win);
	void gui_render(sf::RenderWindow& win);
	void help_marker(char const* desc);
	void export_layer_texture();
	void center_map();
	void launch_demo(char** argv, int room_id, std::filesystem::path path, sf::Vector2<float> player_position);
	[[nodiscard]] auto control_pressed() const -> bool { return pressed_keys.test(PressedKeys::control); }
	[[nodiscard]] auto shift_pressed() const -> bool { return pressed_keys.test(PressedKeys::shift); }
	[[nodiscard]] auto left_mouse_pressed() const -> bool { return pressed_keys.test(PressedKeys::mouse_left); }
	[[nodiscard]] auto right_mouse_pressed() const -> bool { return pressed_keys.test(PressedKeys::mouse_right); }
	[[nodiscard]] auto any_mouse_pressed() const -> bool { return left_mouse_pressed() || right_mouse_pressed(); }
	[[nodiscard]] auto space_pressed() const -> bool { return pressed_keys.test(PressedKeys::space); }
	[[nodiscard]] auto palette_mode() const -> bool { return flags.test(GlobalFlags::palette_mode); }
	[[nodiscard]] auto available() const -> bool { return !window_hovered && !menu_hovered && !popup_open; }

	Canvas map{SelectionType::canvas};
	Canvas palette{SelectionType::palette};

	std::vector<sf::Texture> tileset_textures{};
	sf::Texture tool_texture{};

	sf::RectangleShape wallpaper{};
	sf::RectangleShape target_shape{};
	sf::RectangleShape selector{};

	// for loading out layer pngs
	sf::RenderTexture screencap{};

	sf::Vector2<float> mouse_clicked_position{};

	bool mouse_held{};
	bool show_palette{true};
	bool show_overlay{};
	bool demo_mode{};

	int large_index_multiplier{100};
	int small_index_multiplier{200};

	bool trigger_demo{false};
	bool window_hovered{};
	bool menu_hovered{};
	bool popup_open{};
	int active_layer{4};
	uint32_t selected_block{};

  private:
	WindowManager* window;
	data::ResourceFinder* finder;
	PopupHandler popup{};
	std::unique_ptr<Tool> current_tool;
	std::unique_ptr<Tool> secondary_tool;
	util::BitFlags<PressedKeys> pressed_keys{};
	util::BitFlags<GlobalFlags> flags{};
	char** args{};
	Console console{};
	struct {
		sf::Color backdrop{};
	} colors{};
	struct {
		bool pervasive{};
		bool contiguous{};
	} tool_flags{};
	float zoom_factor{0.05f};
};

} // namespace pi
