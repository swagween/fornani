
#pragma once

#include <imgui.h>
#include <SFML/Graphics.hpp>
#include <editor/automa/EditorState.hpp>
#include <chrono>
#include <cstdio>
#include <filesystem>
#include <memory>
#include <sstream>
#include <string_view>
#include "editor/automa/PopupHandler.hpp"
#include "editor/canvas/Canvas.hpp"
#include "editor/canvas/Clipboard.hpp"
#include "editor/gui/Console.hpp"
#include "editor/setup/WindowManager.hpp"
#include "editor/tool/Tool.hpp"
#include "editor/util/BitFlags.hpp"
#include "fornani/utils/Cooldown.hpp"
#include <imgui-SFML.h>

namespace pi {

enum class GlobalFlags { shutdown, palette_mode };
enum class EditorFlags { create_new_room };

constexpr static std::uint8_t max_layers_v{32};

class Editor final : public EditorState {
  public:
	Editor(fornani::automa::ServiceProvider& svc, EditorContext& ctx);
	EditorStateType run(char** argv) override;
	void handle_events(std::optional<sf::Event> event, sf::RenderWindow& win) override;
	void logic() override;
	void render(sf::RenderWindow& win) override;
	void gui_render(sf::RenderWindow& win);
	void load();
	void load_file(std::string_view to_region, std::string_view to_room);
	void new_file(int id);
	bool save();
	void help_marker(char const* desc);
	void export_layer_texture();
	void center_map();
	void launch_demo(char** argv, int room_id, std::filesystem::path path, sf::Vector2f player_position);
	void reset_layers();
	void delete_current_layer();
	[[nodiscard]] auto palette_mode() const -> bool { return flags.test(GlobalFlags::palette_mode); }
	[[nodiscard]] auto is_any_widget_hovered() const -> bool { return ImGui::GetIO().WantCaptureMouse || (m_options.palette && palette.hovered()) || hazard_hovered; }
	[[nodiscard]] auto hazard_mode() const -> bool { return current_tool->is_mode(BrushMode::hazard); }

	Canvas map;
	Canvas palette;

	std::vector<sf::Texture> tileset_textures{};

	sf::RectangleShape target_shape{};
	sf::RectangleShape selector{};

	// for loading out layer pngs
	sf::RenderTexture screencap{};

	bool mouse_held{};
	bool show_overlay{};
	bool demo_mode{};

	int large_index_multiplier{100};
	int small_index_multiplier{200};

	bool hazard_hovered{};
	std::size_t active_layer{};
	std::uint32_t selected_block{};

	util::BitFlags<EditorFlags> editor_flags{};

  private:
	void set_new_room();

  private:
	sf::Sprite m_tool_sprite;
	PopupHandler popup{};
	std::optional<Clipboard> m_clipboard{};
	std::unique_ptr<Tool> current_tool;
	std::unique_ptr<Tool> secondary_tool;
	util::BitFlags<GlobalFlags> flags{};
	BrushMode m_mode{};
	Console console{};
	struct {
		bool pervasive{};
		bool contiguous{};
	} tool_flags{};
	float zoom_factor{0.05f};
	fornani::util::Cooldown grid_refresh{};
	struct {
		std::string layer_str[max_layers_v];
		char const* layers[max_layers_v];
	} m_labels{};
	struct {
		bool sidebar{true};
		bool console{true};
		bool palette{true};
	} m_options{};
	struct {
		bool fullscreen{};
		bool trigger_demo{};
		bool custom_position{};
	} m_demo{};
	int m_middleground{};

	float m_menu_alpha;

	fornani::automa::ServiceProvider* m_services;
};

} // namespace pi
