#pragma once
#include <SFML/Graphics.hpp>
#include "../utils/BitFlags.hpp"
#include "../utils/NineSlice.hpp"
#include "../utils/Circuit.hpp"
#include <string_view>

namespace automa {
struct ServiceProvider;
struct Option;
}

namespace gui {
enum class MiniMenuState { open };
class MiniMenu {
  public:
	MiniMenu(automa::ServiceProvider& svc, std::vector<std::string_view> opt);
	void update(automa::ServiceProvider& svc, sf::Vector2<float> dim, sf::Vector2<float> position);
	void render(sf::RenderWindow& win) const;
	void open(automa::ServiceProvider& svc, sf::Vector2<float> position);
	void close(automa::ServiceProvider& svc);
	void set_origin(sf::Vector2<float> origin);
	void up();
	void down();
	sf::Vector2<float> get_dimensions() const;
	[[nodiscard]] auto is_open() const -> bool { return state.test(MiniMenuState::open); }
	[[nodiscard]] auto get_selection() const -> int { return selection.get(); }
	util::BitFlags<MiniMenuState> state{};
	sf::Vector2<float> position{};
	sf::Vector2<float> draw_position{};
	std::vector<automa::Option> options{};
  private:
	sf::Vector2<float> dimensions{};
	int maximum{};
	int index{};
	float corner{56.f};
	float edge{2.f};
	util::NineSlice sprite{};
	util::Circuit selection{1};
	sf::Font font{};
};

} // namespace gui
