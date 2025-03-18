
#pragma once

#include "fornani/gui/ItemWidget.hpp"
#include "fornani/gui/Portrait.hpp"
#include "fornani/gui/ResponseDialog.hpp"
#include "fornani/gui/TextWriter.hpp"
#include "fornani/io/Logger.hpp"
#include "fornani/utils/BitFlags.hpp"
#include "fornani/utils/NineSlice.hpp"
#include "fornani/utils/RectPath.hpp"

#include <SFML/Graphics.hpp>

#include <memory>
#include <string>

namespace fornani::config {
class ControllerMap;
}

namespace fornani::gui {

enum class ConsoleMode : std::uint8_t { writing, responding, off };
enum class ConsoleFlags : std::uint8_t { portrait_included, display_item };
enum class OutputType : std::uint8_t { instant, gradual };

enum class MessageCodeType : std::uint8_t { none, response, item, quest, voice, emotion, redirect, action, exit };
enum class CodeSource : std::uint8_t { suite, response };

struct MessageCode {
	CodeSource source{};
	int set{};
	int index{};
	MessageCodeType type{};
	int value{};
	std::optional<std::vector<int>> extras{};
	void debug();
	[[nodiscard]] auto is_exit() const -> bool { return type == MessageCodeType::exit; }
	[[nodiscard]] auto is_response() const -> bool { return source == CodeSource::suite && type == MessageCodeType::response; }
	[[nodiscard]] auto is_redirect() const -> bool { return source == CodeSource::suite && type == MessageCodeType::redirect; }
	[[nodiscard]] auto is_suite_return() const -> bool { return source == CodeSource::response && type == MessageCodeType::response; }
	[[nodiscard]] auto is_action() const -> bool { return type == MessageCodeType::action; }
};

class Console {
  public:
	explicit Console(automa::ServiceProvider& svc);
	/// <summary>
	/// @brief for standard loading and launching, data-driven text
	/// </summary>
	/// <param name="svc"></param>
	/// <param name="key"></param>
	/// <param name="type"></param>
	explicit Console(automa::ServiceProvider& svc, dj::Json& source, std::string_view key, OutputType type);
	/// <summary>
	/// @brief used for loading single messages (signs, inspectables, etc.)
	/// </summary>
	/// <param name="svc"></param>
	/// <param name="message"></param>
	explicit Console(automa::ServiceProvider& svc, std::string_view message);

	void update(automa::ServiceProvider& svc);
	void render(sf::RenderWindow& win);

	void set_source(dj::Json& json);
	void set_nani_sprite(sf::Sprite const& sprite);
	void handle_actions(int value);
	void display_item(int item_id);
	void display_gun(int gun_id);
	void write(sf::RenderWindow& win, bool instant);
	void write(sf::RenderWindow& win);
	void append(std::string_view key);
	void end();
	void include_portrait(int id);

	std::string get_key() const;

	[[nodiscard]] auto is_active() const -> bool { return m_mode == ConsoleMode::writing || m_mode == ConsoleMode::responding; }
	[[nodiscard]] auto is_complete() const -> bool { return !is_active(); }
	[[nodiscard]] auto exit_requested() const -> bool { return m_mode == ConsoleMode::off; }
	[[nodiscard]] auto just_began() const -> bool { return m_began; }
	[[nodiscard]] auto get_message_code() const -> MessageCode;
	[[nodiscard]] auto get_response_code(int which) const -> MessageCode;

	util::RectPath m_path;

	dj::Json text_suite{};

	ItemWidget item_widget;

	automa::ServiceProvider* m_services;

	std::string native_key{};

  protected:
	void load_and_launch(std::string_view key, OutputType type = OutputType::gradual);
	void load_single_message(std::string_view message);
	void handle_inputs(config::ControllerMap& controller);
	void debug();

	std::unique_ptr<TextWriter> m_writer;
	std::optional<ResponseDialog> m_response{};
	std::vector<MessageCode> m_codes{};

	util::BitFlags<ConsoleFlags> m_flags{};

	Portrait m_npc_portrait;
	Portrait m_nani_portrait;

	sf::Vector2f m_position{};
	sf::Vector2f m_dimensions{};
	sf::Vector2f m_response_offset;
	OutputType m_output_type{};
	ConsoleMode m_mode{};
	io::Logger m_logger{"gui"};
	struct {
		int corner_factor{};
		int edge_factor{};
		float padding_scale{};
	} m_styling{};
	util::NineSlice m_nineslice;
	bool m_began{};
};

} // namespace fornani::gui
