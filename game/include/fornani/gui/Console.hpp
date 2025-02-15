
#pragma once

#include "ItemWidget.hpp"
#include "Portrait.hpp"
#include "fornani/gui/TextWriter.hpp"
#include "fornani/utils/BitFlags.hpp"
#include "fornani/utils/NineSlice.hpp"
#include "fornani/utils/RectPath.hpp"

#include <SFML/Graphics.hpp>

#include <memory>
#include <string>

namespace fornani::gui {

constexpr int corner_factor{56};
constexpr int edge_factor{2};
constexpr float height_factor{3.0f};

constexpr float pad{168.f};
constexpr float text_pad{8.0f};

enum class ConsoleFlags : uint8_t { active, selection_mode, portrait_included, off_trigger, extended, display_item, exited };
enum class OutputType { instant, gradual };

struct Border {
	float left{};
	float right{};
	float top{};
	float bottom{};
};

class Console {

  public:
	explicit Console(automa::ServiceProvider& svc);

	void begin();
	void update(automa::ServiceProvider& svc);
	void render(sf::RenderWindow& win);

	void set_source(dj::Json& json);
	void set_texture(sf::Texture& tex);
	void load_and_launch(std::string_view key, OutputType type = OutputType::gradual);
	void load_single_message(std::string_view message);
	void display_item(int item_id);
	void display_gun(int gun_id);
	void write(sf::RenderWindow& win, bool instant);
	void write(sf::RenderWindow& win);
	void append(std::string_view key);
	void end();
	void end_tick();
	void clean_off_trigger();
	void include_portrait(int id);

	std::string get_key();

	[[nodiscard]] auto active() const -> bool { return flags.test(ConsoleFlags::active); }
	[[nodiscard]] auto is_complete() const -> bool { return !static_cast<bool>(writer); }
	[[nodiscard]] auto extended() const -> bool { return m_nineslice.is_extended(); }
	[[nodiscard]] auto off() const -> bool { return flags.test(ConsoleFlags::off_trigger); }
	[[nodiscard]] auto exited() const -> bool { return flags.test(ConsoleFlags::exited); }
	[[nodiscard]] auto consume_exited() -> bool { return flags.consume(ConsoleFlags::exited); }

	sf::Vector2<float> position{};
	sf::Vector2<float> dimensions{};
	sf::Vector2<float> text_origin{};
	util::BitFlags<ConsoleFlags> flags{};
	util::NineSlice m_nineslice;
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

	Border border{48.f, 40.f, 26.f, 26.f};

  protected:
	sf::Vector2<float> origin{}; // bottom left corner
	OutputType m_output_type{};
};

} // namespace fornani::gui
