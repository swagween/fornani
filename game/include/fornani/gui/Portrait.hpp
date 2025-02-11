
#pragma once
#include "fornani/particle/Gravitator.hpp"
#include "fornani/utils/BitFlags.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::gui {
enum class PortraitFlags : uint8_t { custom };
constexpr float pad_x{20.f};
constexpr float pad_y{20.f};
class Portrait {
  public:
	explicit Portrait(automa::ServiceProvider& svc, bool left = true);
	void update(automa::ServiceProvider& svc);
	void set_custom_portrait(sf::Sprite const& sp);
	void render(sf::RenderWindow& win);
	void reset(automa::ServiceProvider& svc);
	void set_position(sf::Vector2<float> pos);
	void bring_in();
	void send_out();
	void set_emotion(int new_emotion);
	void set_id(int new_id);
	[[nodiscard]] auto get_emotion() const -> int { return emotion; }

  private:
	sf::Sprite sprite;
	sf::Sprite window;
	std::string_view label{};
	util::BitFlags<PortraitFlags> flags{};
	int id{};
	int emotion{1}; // 1-index to avoid communication errors
	bool is_nani{};
	sf::Vector2<float> dimensions{};
	sf::Vector2<float> position{};
	sf::Vector2<float> start_position{};
	sf::Vector2<float> end_position{};

	vfx::Gravitator gravitator{};
};

} // namespace fornani::gui
