
#pragma once

#include "../GameState.hpp"

namespace automa {

struct Option {
	Option(ServiceProvider& svc, std::string_view lbl);
	sf::Text label{};
	sf::Vector2<float> position{};
	sf::Vector2<float> left_offset{};
	sf::Vector2<float> right_offset{};
	sf::Vector2<float> dot_offset{0.f, 8.f};
	int index{};
	bool highlighted{};
	bool selected{};
	void update(ServiceProvider& svc, int& selection);
};

class OptionsMenu : public automa::GameState {
  public:
	OptionsMenu() = default;
	OptionsMenu(ServiceProvider& svc, player::Player& player, int id = 0);
	void init(ServiceProvider& svc, std::string_view room = "");
	void setTilesetTexture(ServiceProvider& svc, sf::Texture& t);
	void handle_events(ServiceProvider& svc, sf::Event& event);
	void tick_update(ServiceProvider& svc);
	void frame_update(ServiceProvider& svc);
	void render(ServiceProvider& svc, sf::RenderWindow& win);
	void constrain_selection();

	std::vector<Option> options{};
	int current_selection{};
	float spacing{16.f};
};

} // namespace automa
