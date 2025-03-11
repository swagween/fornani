
#pragma once

#include "ItemWidget.hpp"
#include "Portrait.hpp"
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

enum class MessageCodeType : std::uint8_t { none, response, item, quest, voice, emotion };
enum class CodeSource : std::uint8_t { suite, response };

struct MessageCode {
	CodeSource source{};
	int index{};
	MessageCodeType type{};
	int value{};
	std::optional<std::vector<int>> extras{};
};

class Console {

  public:
	explicit Console(automa::ServiceProvider& svc);
	explicit Console(automa::ServiceProvider& svc, std::string const& texture_lookup);

	void begin();
	void update(automa::ServiceProvider& svc);
	void render(sf::RenderWindow& win);

	void set_source(dj::Json& json);
	void load_and_launch(std::string_view key, OutputType type = OutputType::gradual);
	void load_single_message(std::string_view message);
	void display_item(int item_id);
	void display_gun(int gun_id);
	void write(sf::RenderWindow& win, bool instant);
	void write(sf::RenderWindow& win);
	void append(std::string_view key);
	void end();
	void include_portrait(int id);

	std::string get_key();

	[[nodiscard]] auto is_active() const -> bool { return m_mode == ConsoleMode::writing || m_mode == ConsoleMode::responding; }
	[[nodiscard]] auto is_complete() const -> bool { return !is_active(); }
	[[nodiscard]] auto exit_requested() const -> bool { return m_mode == ConsoleMode::off; }
	// [[nodiscard]] auto get_message_code() const -> MessageCode { return m_codes.back(); }

	util::BitFlags<ConsoleFlags> flags{};
	util::RectPath m_path;

	dj::Json text_suite{};

	Portrait portrait;
	Portrait nani_portrait;
	sf::Texture nani_texture{};
	ItemWidget item_widget;

	automa::ServiceProvider* m_services;

	std::unique_ptr<TextWriter> writer;
	std::string native_key{};

	struct {
		int out_voice{};
		int out_emotion{};
	} communicators{};

  protected:
	void handle_inputs(config::ControllerMap& controller);
	void debug();

	std::vector<MessageCode> m_codes{};

	sf::Vector2<float> position{};
	sf::Vector2<float> dimensions{};
	OutputType m_output_type{};
	ConsoleMode m_mode{};
	io::Logger m_logger{"Console"};
	struct {
		int corner_factor{};
		int edge_factor{};
		float padding_scale{};
	} m_styling{};
	util::NineSlice m_nineslice;
};

} // namespace fornani::gui
