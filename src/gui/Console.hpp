
#pragma once

#include <SFML/Graphics.hpp>
#include <array>
#include <string>
#include "../utils/BitFlags.hpp"
#include "../graphics/TextWriter.hpp"
#include "Portrait.hpp"

namespace gui {

int const corner_factor{56};
int const edge_factor{2};
float const height_factor{3.0f};

float const pad{168.f};
float const text_pad{8.0f};

enum class ConsoleFlags { active, loaded, selection_mode, portrait_included, off_trigger, extended };

struct Border {
	float left{};
	float right{};
	float top{};
	float bottom{};
};

class Console {

  public:
	Console() = default;
	Console(automa::ServiceProvider& svc);

	void begin();
	void update(automa::ServiceProvider& svc);
	void render(sf::RenderWindow& win);

	void set_source(dj::Json& json);
	void set_texture(sf::Texture& tex);
	void load_and_launch(std::string_view key);
	void write(sf::RenderWindow& win, bool instant = true);
	void end();
	void end_tick();
	void clean_off_trigger();
	void include_portrait(int id);

	void nine_slice(int corner_dim, int edge_dim);

	[[nodiscard]] auto active() const -> bool { return flags.test(ConsoleFlags::active); }
	[[nodiscard]] auto is_complete() const -> bool { return writer.empty(); }
	[[nodiscard]] auto extended() const -> bool { return flags.test(ConsoleFlags::extended); }
	[[nodiscard]] auto off() const -> bool { return flags.test(ConsoleFlags::off_trigger); }

	sf::Vector2<float> position{};
	sf::Vector2<float> current_dimensions{};
	sf::Vector2<float> final_dimensions{};
	sf::Vector2<float> text_origin{};
	util::BitFlags<ConsoleFlags> flags{};

	std::array<sf::Sprite, 9> sprites{};

	dj::Json text_suite{};

	gui::Portrait portrait;
	gui::Portrait nani_portrait;

	automa::ServiceProvider* m_services;

	text::TextWriter writer;

	struct {
		int out_voice{};
		int out_emotion{};
	} communicators{};

	Border border{
		48.f,
		40.f,
		26.f,
		26.f
	};

	float extent{};
	int speed{2};

	protected:
	sf::Vector2<float> origin{}; // bottom left corner
};

} // namespace gui
