
#pragma once

#include <assert.h>
#include <djson/json.hpp>
#include <cmath>
#include <list>
#include <queue>
#include <random>
#include <unordered_map>
#include <vector>
#include "../../components/BehaviorComponent.hpp"
#include "../../components/PhysicsComponent.hpp"
#include "../../setup/ServiceLocator.hpp"
#include "../../utils/BitFlags.hpp"
#include "../../utils/Collider.hpp"
#include "../../utils/StateFunction.hpp"

namespace critter {

enum class VARIANT { BEAST, SOLDIER, GRUB, GHOST };

enum class CRITTER_TYPE { hulmet, tank, bunker, thug, worker, frdog, jackdaw, frcat, biter, bee, roller, snek, behemoth, stinger, watchman, gnat, moth };

inline std::unordered_map<int, sf::Texture&> get_critter_texture{{0, svc::assetLocator.get().t_frdog}, {1, svc::assetLocator.get().t_hulmet}};

struct CritterMetadata {
	int id{};
	VARIANT variant{};
	bool hostile{};
	bool hurt_on_contact{};
	bool gravity{};
};

struct CritterStats {
	int base_damage{};
	float base_hp{};
	float speed{};
	float loot_multiplier{};
	int energy{};
	int vision{};
	int cooldown{};
};

struct CritterCondition {
	float hp{1};
};

enum class Flags { alive, seeking, awake, awakened, asleep, turning, flip, barking, hurt, just_hurt, shot, vulnerable, charging, shooting, hiding, running, weapon_fired };

struct FrameTracker {
	int frame{};
};

class Critter {

  public:

	Critter() = default;
	Critter(CritterMetadata m, CritterStats s, sf::Vector2<int> sprite_dim, sf::Vector2<int> spritesheet_dim, sf::Vector2<float> dim)
		: metadata(m), stats(s), sprite_dimensions(sprite_dim), spritesheet_dimensions(spritesheet_dim), dimensions(dim) {
		
		set_sprite();

		alert_range = shape::Shape({(float)s.vision * 1.5f, (float)s.vision * 1.5f});
		hostile_range = shape::Shape({(float)s.vision, (float)s.vision});

		ar.setSize({(float)(s.vision * 1.5), (float)(s.vision * 1.5)});
		hr.setSize({(float)s.vision, (float)s.vision});
		condition.hp = s.base_hp;
	}
	~Critter() {}

	virtual void unique_update(){};
	virtual void load_data(){};
	virtual void sprite_flip();
	virtual void cooldown(); // for armed enemies

	void init();
	void update();
	void render(sf::RenderWindow& win, sf::Vector2<float> campos);
	void set_sprite();
	void set_position(sf::Vector2<int> pos);
	void seek_current_target();
	void wake_up();
	void sleep();
	void awake();

	// general critter methods, to be called dependent on critter type
	//    void stop();
	//    void reverse_direction();
	void random_idle_action();
	void random_walk(sf::Vector2<int> range);
	//    void attack();
	//    void defend();
	//    void hop();
	//    void leap();
	//    void evade(sf::Vector2<float>& stimulus_pos);
	//    void hover_sine();
	//    void fly_sine();
	//    void fly_line(bool vertical);

	CritterMetadata metadata{};
	CritterStats stats{};
	CritterCondition condition{};
	util::BitFlags<Flags> flags{};

	behavior::Behavior behavior{};
	dir::Direction direction{};
	std::vector<shape::Collider> colliders{};

	std::vector<shape::Shape> hurtbox_atlas{};
	std::vector<shape::Shape> hurtboxes{};
	shape::Shape alert_range{};
	shape::Shape hostile_range{};

	sf::Vector2<int> sprite_dimensions{};
	sf::Vector2<int> spritesheet_dimensions{};
	sf::Vector2<float> offset{};
	sf::Vector2<float> dimensions{};
	sf::Vector2<float> sprite_position{};
	sf::Vector2<float> barrel_point{};

	arms::Weapon weapon{};

	int anim_loop_count{};

	// collider loading
	int num_anim_frames{};
	int num_colliders{};
	int num_hurtboxes{};

	sf::Sprite sprite{};
	sf::RectangleShape drawbox{}; // for debugging
	sf::RectangleShape ar{};
	sf::RectangleShape hr{};
	sf::RectangleShape hpbox{}; // for debug

	std::queue<int> idle_action_queue{};

	sf::Vector2<float> current_target{};

	int unique_id{};
};

} // namespace critter
