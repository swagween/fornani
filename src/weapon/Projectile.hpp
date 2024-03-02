
#pragma once

#include <chrono>
#include <cstdio>
#include <deque>
#include <list>
#include <memory>
#include <unordered_map>
#include "../components/PhysicsComponent.hpp"
#include "../entities/behavior/Animation.hpp"
#include "../graphics/FLColor.hpp"
#include "../particle/Emitter.hpp"
#include "../utils/BitFlags.hpp"
#include "../utils/Direction.hpp"
#include "../utils/Random.hpp"
#include "../utils/Shape.hpp"

namespace arms {

enum class WEAPON_TYPE {
	BRYNS_GUN,
	PLASMER,
	WASP,
	BLIZZARD,
	BISMUTH,
	UNDERDOG,
	ELECTRON,
	CLOVER,
	TRITON,
	WILLET_585,
	QUASAR,
	NOVA,
	VENOM,
	TWIN,
	CARISE,
	STINGER,
	TUSK,
	TOMAHAWK,

	SKYCORPS_AR
};

enum class TEAMS { NANI, SKYCORPS, BEASTS };

enum class RENDER_TYPE { ANIMATED, SINGLE_SPRITE, MULTI_SPRITE };

sf::Vector2<float> const DEFAULT_DIMENSIONS{8.0, 8.0};
int const history_limit{4};

struct ProjectileStats {

	int base_damage{};
	int range{};

	float speed{};
	float variance{};
	float stun_time{};
	float knockback{};

	bool persistent{};

	float acceleration_factor{};
	float dampen_factor{};

	int range_variance{};
};

struct ProjectileAnimation {
	int num_sprites{};
	int num_frames{};
	int framerate{};
};

enum class ProjectileState { initialized, destruction_initiated, destroyed };

class Projectile {

  public:
	Projectile();
	Projectile(int id);

	void update();
	void render(sf::RenderWindow& win, sf::Vector2<float>& campos);
	void destroy(bool completely);
	void seed();
	void set_sprite();
	void set_orientation(sf::Sprite& sprite);
	void set_position(sf::Vector2<float>& pos);
	void sync_position();
	void constrain_sprite_at_barrel(sf::Sprite& sprite, sf::Vector2<float>& campos);
	void constrain_sprite_at_destruction_point(sf::Sprite& sprite, sf::Vector2<float>& campos);
	void constrain_hitbox_at_barrel();
	void constrain_hitbox_at_destruction_point();

	dir::Direction direction{};
	shape::Shape bounding_box{};
	components::PhysicsComponent physics{};
	ProjectileStats stats{};
	ProjectileAnimation anim{};

	WEAPON_TYPE type{WEAPON_TYPE::BRYNS_GUN};
	TEAMS team{TEAMS::NANI}; // default to player projectile. enemies and bosses will be set separately
	RENDER_TYPE render_type{};

	util::BitFlags<ProjectileState> state{};

	sf::Vector2<float> max_dimensions{};
	sf::Vector2<float> fired_point{};
	sf::Vector2<float> destruction_point{};

	std::vector<sf::Sprite> sp_proj{};

	int sprite_id{};
	int curr_frame{};
	int anim_frame{};

	sf::RectangleShape box{};

	std::vector<sf::Color> colors{};
	std::deque<sf::Vector2<float>> position_history{};
};
} // namespace arms
