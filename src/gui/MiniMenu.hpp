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
	void open(sf::Vector2<float> position);
	void close() { state.reset(MiniMenuState::open); };
	void set_origin(sf::Vector2<float> origin);
	void up();
	void down();
	sf::Vector2<float> get_dimensions() const;
	[[nodiscard]] auto is_open() const -> bool { return state.test(MiniMenuState::open); }
	util::BitFlags<MiniMenuState> state{};
	sf::Vector2<float> position{};
	sf::Vector2<float> draw_position{};
  private:
	sf::Vector2<float> dimensions{};
	int maximum{};
	int index{};
	int corner{56};
	int edge{2};
	util::NineSlice sprite{};
	std::vector<automa::Option> options{};
	util::Circuit selection{0};
};

} // namespace gui
