#pragma once
#include <SFML/Graphics.hpp>
#include "fornani/utils/BitFlags.hpp"
#include "fornani/utils/NineSlice.hpp"
#include "fornani/utils/Circuit.hpp"
#include "fornani/automa/Option.hpp"
#include <string_view>

namespace automa {
struct ServiceProvider;
}

namespace gui {
enum class MiniMenuState { open };
class MiniMenu {
  public:
	MiniMenu() = default;
	MiniMenu(automa::ServiceProvider& svc, std::vector<std::string_view> opt, bool white = false);
	void overwrite_option(int idx, std::string_view replacement);
	void update(automa::ServiceProvider& svc, sf::Vector2<float> dim, sf::Vector2<float> at_position);
	void render(sf::RenderWindow& win, bool bg = true);
	void open(automa::ServiceProvider& svc, sf::Vector2<float> at_position);
	void close(automa::ServiceProvider& svc);
	void set_origin(sf::Vector2<float> origin);
	void up(automa::ServiceProvider& svc);
	void down(automa::ServiceProvider& svc);
	void speed_up_appearance(int rate) { sprite.speed_up_appearance(rate); }
	void set_force(float force) { sprite.set_force(force); }
	void set_fric(float fric) { sprite.set_fric(fric); }
	sf::Vector2<float> get_dimensions() const;
	[[nodiscard]] auto is_open() const -> bool { return state.test(MiniMenuState::open); }
	[[nodiscard]] auto get_selection() const -> int { return selection.get(); }
	util::BitFlags<MiniMenuState> state{};
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

} // namespace gui
