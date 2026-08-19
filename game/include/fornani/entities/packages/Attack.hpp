
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/components/CircleSensor.hpp>
#include <fornani/core/Fwd.hpp>

namespace fornani::player {
class Player;
enum class PlayerDeathType : std::uint8_t;
} // namespace fornani::player

namespace fornani::entity {

struct Attack {
	void update();
	void set_position(sf::Vector2f position);
	void handle_player(player::Player& player);
	bool hurt_player(player::Player& player, float damage = 1.f, sf::Vector2f knockback = {});
	bool kill_player(player::Player& player, player::PlayerDeathType death, bool center_only = false);
	void cancel_projectiles(automa::ServiceProvider& svc, world::Map& map, arms::Team team, float freezeframe = 0.02f);
	void set_constant_radius(float to);
	void render(sf::RenderWindow& win, sf::Vector2f cam);
	void enable();
	void disable();

	components::CircleSensor sensor{};
	components::CircleSensor hit{};
	sf::Vector2f origin{};
	sf::Vector2f hit_offset{};
};

} // namespace fornani::entity
