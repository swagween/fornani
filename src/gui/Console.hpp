
#pragma once

#include <SFML/Graphics.hpp>
#include <array>
#include <string>
#include "../utils/BitFlags.hpp"
#include "../utils/Camera.hpp"
#include "../graphics/TextWriter.hpp"
#include "Portrait.hpp"

namespace gui {

int const corner_factor{56};
int const edge_factor{2};
float const height_factor{3.0f};

float const pad{168.f};
float const text_pad{8.0f};

enum class ConsoleFlags { active, loaded, selection_mode };

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
	void load_and_launch(std::string_view key);
	void write(sf::RenderWindow& win, bool instant = true);
	void end();

	void nine_slice(int corner_dim, int edge_dim);

	sf::Vector2<float> position{};
	sf::Vector2<float> current_dimensions{};
	sf::Vector2<float> final_dimensions{};
	sf::Vector2<float> text_origin{};
	util::BitFlags<ConsoleFlags> flags{};

	std::array<sf::Sprite, 9> sprites{};

	dj::Json text_suite{};

	gui::Portrait portrait;

	text::TextWriter writer;
	Border border{
		48.f,
		40.f,
		26.f,
		26.f
	};

	int extent{};
	int speed{2};

	protected:
	sf::Vector2<float> origin{pad, cam::screen_dimensions.y - pad_y}; // bottom left corner
};

} // namespace gui
