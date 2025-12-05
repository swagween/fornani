
#pragma once

#include <fornani/components/PhysicsComponent.hpp>
#include <fornani/components/SteeringBehavior.hpp>
#include <fornani/graphics/Drawable.hpp>
#include <fornani/particle/Sparkler.hpp>
#include <fornani/utils/BitFlags.hpp>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::gui {

enum class PortraitFlags { custom, right };

constexpr float pad_x{20.f};
constexpr float pad_y{20.f};

class Portrait : public Drawable {
  public:
	explicit Portrait(automa::ServiceProvider& svc, sf::Texture const& texture, int id, bool left = true);
	explicit Portrait(automa::ServiceProvider& svc, int id, bool left = true);
	void update(automa::ServiceProvider& svc);
	void render(sf::RenderWindow& win);
	void reset(automa::ServiceProvider& svc);
	void set_position(sf::Vector2f pos);
	void bring_in();
	void send_out();
	void set_emotion(int new_emotion);
	void add_sparkler(std::string_view tag);
	void remove_sparkler();

	[[nodiscard]] auto get_emotion() const -> int { return m_emotion; }

  private:
	sf::Sprite window;
	std::string_view label{};
	util::BitFlags<PortraitFlags> flags{};
	int m_id{};
	int m_emotion{};
	sf::Vector2f dimensions{};
	sf::Vector2f position{};
	sf::Vector2f start_position{};
	sf::Vector2f end_position{};

	std::optional<vfx::Sparkler> m_sparkler{};

	automa::ServiceProvider* m_services;

	components::PhysicsComponent m_physics{};
	components::SteeringBehavior m_steering{};
};

} // namespace fornani::gui
