
#pragma once

#include <fornani/core/Common.hpp>
#include <fornani/gui/ItemWidget.hpp>
#include <fornani/gui/Portrait.hpp>
#include <fornani/gui/console/ResponseDialog.hpp>
#include <fornani/gui/console/TextWriter.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/utils/BitFlags.hpp>
#include <fornani/utils/Cooldown.hpp>
#include <fornani/utils/NineSlice.hpp>
#include <fornani/utils/RectPath.hpp>

#include <SFML/Graphics.hpp>

#include <memory>
#include <string>

namespace fornani::config {
class ControllerMap;
}

namespace fornani::gui {

enum class ConsoleMode { writing, responding, off };
enum class ConsoleFlags { no_exit };
enum class ConsoleTriggers { response_created };
enum class OutputType { gradual, instant, no_exit, no_skip };

class Console {
  public:
	explicit Console(automa::ServiceProvider& svc);

	/// <summary>
	/// @brief for standard loading and launching, data-driven text
	/// </summary>
	/// <param name="svc"></param>
	/// <param name="key"></param>
	/// <param name="type"></param>
	explicit Console(automa::ServiceProvider& svc, dj::Json const& source, std::string_view key, OutputType type);

	/// <summary>
	/// @brief for standard loading and launching, data-driven text with pre-supplied key
	/// </summary>
	/// <param name="svc"></param>
	/// <param name="type"></param>
	explicit Console(automa::ServiceProvider& svc, dj::Json const& source, OutputType type);

	/// <summary>
	/// @brief used for loading single messages (signs, inspectables, etc.)
	/// </summary>
	/// <param name="svc"></param>
	/// <param name="message"></param>
	explicit Console(automa::ServiceProvider& svc, std::string_view message);

	void update(automa::ServiceProvider& svc);
	void render(sf::RenderWindow& win);

	void set_source(dj::Json const& json);
	void set_nani_sprite(sf::Sprite const& sprite);
	void set_no_exit(bool flag) { flag ? m_flags.set(ConsoleFlags::no_exit) : m_flags.reset(ConsoleFlags::no_exit); }
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
	[[nodiscard]] auto get_message_codes() const -> std::optional<std::vector<MessageCode>>;
	[[nodiscard]] auto get_response_codes(int which) const -> std::optional<std::vector<MessageCode>>;
	[[nodiscard]] auto has_nani_portrait() const -> bool { return static_cast<bool>(m_nani_portrait); }
	[[nodiscard]] auto was_response_created() const -> bool { return m_triggers.test(ConsoleTriggers::response_created); }
	[[nodiscard]] auto can_exit() const -> bool { return m_output_type != OutputType::no_skip && m_output_type != OutputType::no_exit; }

	util::RectPath m_path;
	dj::Json text_suite{};
	automa::ServiceProvider* m_services;

	std::string native_key{};

  protected:
	void load_and_launch(std::string_view key, OutputType type = OutputType::gradual);
	void load_and_launch(OutputType type = OutputType::gradual);
	void load_single_message(std::string_view message);
	void handle_inputs(config::ControllerMap& controller);
	void debug();

	std::unique_ptr<TextWriter> m_writer;
	std::optional<ResponseDialog> m_response{};
	std::optional<ItemWidget> m_item_widget{};
	std::optional<Portrait> m_npc_portrait;
	std::optional<Portrait> m_nani_portrait;

	util::BitFlags<ConsoleFlags> m_flags{};
	util::BitFlags<ConsoleTriggers> m_triggers{};
	util::Cooldown m_exit_stall;

	sf::Vector2f m_position{};
	sf::Vector2f m_dimensions{};
	sf::Vector2f m_response_offset;

	OutputType m_output_type{};
	ConsoleMode m_mode{};

	struct {
		int corner_factor{};
		int edge_factor{};
		float padding_scale{};
	} m_styling{};

	util::NineSlice m_nineslice;

	bool m_began{};
	bool m_process_codes{};
	bool m_process_code_before{};
	bool m_process_code_after{};

	io::Logger m_logger{"gui"};
};

} // namespace fornani::gui
