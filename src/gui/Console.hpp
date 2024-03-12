
#pragma once

#include <SFML/Graphics.hpp>
#include <array>
#include <string>
#include "../utils/BitFlags.hpp"
#include "../utils/Camera.hpp"
#include "../graphics/TextWriter.hpp"

namespace automa {
struct ServiceProvider;
}

namespace gui {

int const corner_factor{56};
int const edge_factor{2};
float const height_factor{3.0f};

float const pad{168.f};
float const pad_y{20};
float const text_pad{8.0f};
inline const sf::Vector2<float> origin{pad, cam::screen_dimensions.y - pad_y}; // bottom left corner

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
	void update();
	void render(sf::RenderWindow& win);

	void load_and_launch(std::string_view key);
	void write(sf::RenderWindow& win, bool instant = true);
	void end();

	void nine_slice(int corner_dim, int edge_dim);

	sf::Vector2<float> position{};
	sf::Vector2<float> dimensions{};
	sf::Vector2<float> text_origin{};
	util::BitFlags<ConsoleFlags> flags{};

	std::array<sf::Sprite, 9> sprites{};

	dj::Json text_suite{};

	text::TextWriter writer;
	Border border{
		48.f,
		40.f,
		26.f,
		26.f
	};

	int extent{};
	int speed{2};
};

} // namespace gui
