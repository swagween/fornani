
#pragma once

#include "fornani/components/PhysicsComponent.hpp"
#include "fornani/components/SteeringBehavior.hpp"
#include "fornani/utils/BitFlags.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::gui {

enum class PortraitFlags : std::uint8_t { custom };

constexpr float pad_x{20.f};
constexpr float pad_y{20.f};

class Portrait {
  public:
	explicit Portrait(automa::ServiceProvider& svc, bool left = true);
	void update(automa::ServiceProvider& svc);
	void set_custom_portrait(sf::Sprite const& sp);
	void render(sf::RenderWindow& win);
	void reset(automa::ServiceProvider& svc);
	void set_position(sf::Vector2f pos);
	void set_texture(sf::Texture const& texture);
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
	sf::Vector2f dimensions{};
	sf::Vector2f position{};
	sf::Vector2f start_position{};
	sf::Vector2f end_position{};

	components::PhysicsComponent m_physics{};
	components::SteeringBehavior m_steering{};
};

} // namespace fornani::gui
