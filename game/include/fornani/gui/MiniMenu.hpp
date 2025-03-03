#pragma once
#include <SFML/Graphics.hpp>
#include <string_view>
#include "fornani/automa/Option.hpp"
#include "fornani/utils/BitFlags.hpp"
#include "fornani/utils/Circuit.hpp"
#include "fornani/utils/NineSlice.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::gui {
class MiniMenu {
  public:
	MiniMenu(automa::ServiceProvider& svc, std::vector<std::string_view> opt, sf::Vector2f start_position, bool white = false);
	void update(automa::ServiceProvider& svc, sf::Vector2<float> dim, sf::Vector2<float> at_position);
	void render(sf::RenderWindow& win, bool bg = true);
	void set_origin(sf::Vector2<float> origin);
	void up(automa::ServiceProvider& svc);
	void down(automa::ServiceProvider& svc);
	void speed_up_appearance(int const rate) { sprite.speed_up_appearance(rate); }
	void set_force(float const force) { sprite.set_force(force); }
	void set_fric(float const fric) { sprite.set_fric(fric); }
	sf::Vector2<float> get_dimensions() const;
	[[nodiscard]] auto get_selection() const -> int { return selection.get(); }
	sf::Vector2<float> position{};
	sf::Vector2<float> draw_position{};

  private:
	sf::Vector2<float> dimensions{};
	int maximum{};
	int index{};
	float corner{56.f};
	float edge{2.f};
	util::NineSlice sprite;
	util::Circuit selection{1};
	std::vector<automa::Option> options;
};

} // namespace fornani::gui
