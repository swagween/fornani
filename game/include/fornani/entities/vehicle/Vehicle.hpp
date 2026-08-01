#pragma once

#include <fornani/components/SteeringBehavior.hpp>
#include <fornani/core/Common.hpp>
#include <fornani/core/Fwd.hpp>
#include <fornani/entities/Mobile.hpp>
#include <fornani/graphics/Animatable.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/utils/BitFlags.hpp>

namespace fornani {

enum class VehicleFlags { airborne, idling };

struct VehiclePart {
	VehiclePart(automa::ServiceProvider& svc, dj::Json const& in, sf::Vector2i dimensions);
	Animatable piece;
	DrawOrder order;
};

class Vehicle : public Mobile {
  public:
	Vehicle(automa::ServiceProvider& svc, world::Map& map, std::string_view tag);
	void update(automa::ServiceProvider& svc, world::Map& map, player::Player& player);
	void render(sf::RenderWindow& win, sf::Vector2f cam, DrawOrder order);
	void set_target(sf::Vector2f to) { m_target = to; }
	void set_position(sf::Vector2f to);
	void set_idle(bool to);

	[[nodiscard]] auto is_close_to_target(float const distance) const -> bool { return (get_collider().physics.position - m_target).length() < distance; }
	[[nodiscard]] auto is_close_to_point(sf::Vector2f const point, float const distance) const -> bool { return (get_collider().physics.position - point).length() < distance; }

  private:
	// data-driven
	std::vector<VehiclePart> m_parts{};
	sf::Vector2f m_driver_socket{};
	DrawOrder m_order{};

	// variable
	util::BitFlags<VehicleFlags> m_flags{};
	components::SteeringBehavior m_steering{};
	sf::Vector2f m_target{};
	sf::Vector2f m_tweak{};

	io::Logger m_logger{"Vehicle"};
};

} // namespace fornani
