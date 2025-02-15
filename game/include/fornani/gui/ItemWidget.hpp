
#pragma once
#include "fornani/particle/Gravitator.hpp"
#include "fornani/particle/Sparkler.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::gui {

enum class WidgetFlags : uint8_t { gun };

class ItemWidget {
  public:
	explicit ItemWidget(automa::ServiceProvider& svc);
	void update(automa::ServiceProvider& svc);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win);
	void reset(automa::ServiceProvider& svc);
	void set_position(sf::Vector2<float> pos);
	void bring_in();
	void send_out();
	void set_id(int new_id, bool is_gun = false);

  private:
	struct {
		sf::Sprite item;
		sf::Sprite gun;
	} sprites;
	sf::CircleShape sticker{};

	std::string_view label{};
	int id{};
	util::BitFlags<WidgetFlags> flags{};
	sf::Vector2<float> dimensions{};
	sf::Vector2<float> gun_dimensions{};
	sf::Vector2<float> position{};
	sf::Vector2<float> start_position{};
	sf::Vector2<float> end_position{};
	sf::Vector2<float> pad{};

	vfx::Gravitator gravitator{};
	vfx::Sparkler sparkler{};
};

} // namespace fornani::gui
