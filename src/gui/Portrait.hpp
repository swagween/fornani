
#pragma once
#include <string_view>
#include "../particle/Gravitator.hpp"

namespace automa {
struct ServiceProvider;
}

namespace gui {
float const pad_x{20.f};
float const pad_y{20.f};
class Portrait {
  public:
	Portrait() = default;
	Portrait(automa::ServiceProvider& svc, bool left = true);
	void update(automa::ServiceProvider& svc);
	void render(sf::RenderWindow& win);
	void reset(automa::ServiceProvider& svc);
	void set_position(sf::Vector2<float> pos);
	void bring_in();
	void send_out();
	void set_emotion(int new_emotion);
	void set_id(int new_id);
	[[nodiscard]] auto get_emotion() const -> int { return emotion; }

  private:
	sf::Sprite sprite{};
	std::string_view label{};
	int id{};
	int emotion{1}; // 1-index to avoid communication errors
	bool is_nani{};
	sf::Vector2<float> dimensions{};
	sf::Vector2<float> position{};
	sf::Vector2<float> start_position{};
	sf::Vector2<float> end_position{};

	vfx::Gravitator gravitator{};
};

} // namespace gui
