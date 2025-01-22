
#pragma once

#include <SFML/Graphics.hpp>
#include <string_view>
#include <djson/json.hpp>
#include <filesystem>

namespace pi {

class ResourceFinder;
class Canvas;

struct Entity {
	Entity(std::string label, int id = 0, sf::Vector2<uint32_t> position = {}, sf::Vector2<uint32_t> dimensions = {}) : label(label), id(id), position(position), dimensions(dimensions){};
	virtual ~Entity() = default;
	virtual void serialize(dj::Json& out) {
		out["id"] = id;
		out["position"][0] = position.x;
		out["position"][1] = position.y;
		out["dimensions"][0] = dimensions.x;
		out["dimensions"][1] = dimensions.y;
	}
	virtual void unserialize(dj::Json& in) {
		id = in["id"].as<int>();
		position.x = in["position"][0].as<uint32_t>();
		position.y = in["position"][1].as<uint32_t>();
		dimensions.x = in["dimensions"][0].as<uint32_t>();
		dimensions.y = in["dimensions"][1].as<uint32_t>();
	}
	std::string label{};
	sf::Vector2<uint32_t> position{};
	sf::Vector2<uint32_t> dimensions{};
	int id{};

	// helpers
	sf::RectangleShape drawbox{};
	[[nodiscard]] auto get_label() const -> std::string { return label; }
	[[nodiscard]] auto f_dimensions() const -> sf::Vector2<float> { return sf::Vector2<float>(dimensions); }
};

struct Inspectable : public Entity {
	Inspectable(bool activate_on_contact, std::string key, std::vector<std::vector<std::string>> suites, std::vector<std::vector<std::string>> responses, int alternates)
		: Entity("inspectables", 0, {}, {1, 1}), activate_on_contact(activate_on_contact), key(key), suites(suites), responses(responses), alternates(alternates) {}
	bool activate_on_contact{};
	std::string key{};
	std::vector<std::vector<std::string>> suites{};
	std::vector<std::vector<std::string>> responses{};
	int alternates{};
	void serialize(dj::Json& out) override {
		Entity::serialize(out);
		out["activate_on_contact"] = dj::Boolean{activate_on_contact};
		out["key"] = key;
		out["alternates"] = alternates;
	}
	void unserialize(dj::Json& in) override { Entity::unserialize(in); };
};

struct Critter : public Entity {
	Critter() : Entity("enemies") {}
	void serialize(dj::Json& out) override { Entity::serialize(out); }
	void unserialize(dj::Json& in) override { Entity::unserialize(in); };

	struct Animator : public Entity {
		Animator() : Entity("animators") {}
		bool automatic{};
		bool foreground{};
		int style{};
		void serialize(dj::Json& out) override { Entity::serialize(out); }
		void unserialize(dj::Json& in) override { Entity::unserialize(in); };
	};

	struct Portal : public Entity {
		Portal() : Entity("portals") {}
		bool activate_on_contact{};
		bool already_open{};
		int source_map_id{};
		int destination_map_id{};
		bool locked{};
		int key_id{};
		void serialize(dj::Json& out) override {
			Entity::serialize(out);
			out["activate_on_contact"] = dj::Boolean{activate_on_contact};
			out["already_open"] = dj::Boolean{already_open};
			out["source_map_id"] = source_map_id;
			out["destination_map_id"] = destination_map_id;
			out["locked"] = dj::Boolean{locked};
			out["key_id"] = key_id;
		}
		void unserialize(dj::Json& in) override { Entity::unserialize(in); };
	};

	struct InteractiveScenery : public Entity {
		InteractiveScenery() : Entity("interactive_scenery") {}
		int length{};
		int size{};
		bool foreground{};
		int type{};
		bool has_platform{};
		std::vector<int> link_indeces{};
		void serialize(dj::Json& out) override { Entity::serialize(out); }
		void unserialize(dj::Json& in) override { Entity::unserialize(in); };
	};

	struct NPC : public Entity {
		NPC() : Entity("npcs") {}
		bool background{};
		std::vector<std::vector<std::string>> suites{};
		void serialize(dj::Json& out) override { Entity::serialize(out); }
		void unserialize(dj::Json& in) override { Entity::unserialize(in); };
	};

	struct Platform : public Entity {
		Platform(sf::Vector2u dim, int extent, std::string type, float start) : Entity("platforms", 0, {}, dim), extent(extent), type(type), start(start) {}
		int extent{};
		std::string type{};
		float start{};
		void serialize(dj::Json& out) override {
			Entity::serialize(out);
			out["extent"] = extent;
			out["type"] = type;
			out["start"] = start;
		}
		void unserialize(dj::Json& in) override { Entity::unserialize(in); };
	};

	struct Chest : public Entity {
		Chest() : Entity("chests") {}
		int item_id{};
		int type{};
		float rarity{};
		int amount{};
		void serialize(dj::Json& out) override {
			Entity::serialize(out);
			out["item_id"] = item_id;
			out["type"] = type;
			out["rarity"] = rarity;
			out["amount"] = amount;
		}
		void unserialize(dj::Json& in) override { Entity::unserialize(in); };
	};

	struct Scenery : public Entity {
		Scenery() : Entity("scenery") {}
		int style{};
		int layer{};
		int variant{};
		void serialize(dj::Json& out) override { Entity::serialize(out); }
		void unserialize(dj::Json& in) override { Entity::unserialize(in); };
	};

	struct SwitchBlock : public Entity {
		SwitchBlock() : Entity("switch_blocks") {}
		int type{};
		void serialize(dj::Json& out) override {
			Entity::serialize(out);
			out["type"] = type;
		}
		void unserialize(dj::Json& in) override { Entity::unserialize(in); };
	};

	struct SwitchButton : public Entity {
		SwitchButton(int id, int type) : Entity("switches", id), type(type) {}
		int type{};
		void serialize(dj::Json& out) override {
			Entity::serialize(out);
			out["type"] = type;
		}
		void unserialize(dj::Json& in) override { Entity::unserialize(in); };
	};

	struct Destroyer : public Entity {
		Destroyer() : Entity("destroyers") {}
		void serialize(dj::Json& out) override { Entity::serialize(out); }
		void unserialize(dj::Json& in) override { Entity::unserialize(in); };
	};

	struct SavePoint : public Entity {
		SavePoint() : Entity("save_point") {}
		bool placed{};
		void serialize(dj::Json& out) override { Entity::serialize(out); }
		void unserialize(dj::Json& in) override { Entity::unserialize(in); };
	};

	class EntitySet {
	  public:
		EntitySet() = default;
		EntitySet(ResourceFinder& finder, dj::Json& metadata, std::string const& room_name);
		void render(Canvas& map, sf::RenderWindow& win, sf::Vector2<float> cam);
		void load(ResourceFinder& finder, dj::Json& metadata, std::string const& room_name);
		bool save(ResourceFinder& finder, dj::Json& metadata, std::string const& room_name);
		void clear();
		bool has_entity_at(sf::Vector2<uint32_t> pos) const;

		struct {
			sf::Sprite large_animator{};
			sf::Sprite small_animator{};
			sf::Sprite large_animator_thumb{};
			sf::Sprite small_animator_thumb{};
			sf::Sprite enemy_thumb{};
			sf::Sprite current_enemy{};
		} sprites{};

		struct {
			std::string music{};
			sf::Vector2<uint32_t> player_start{};
			std::vector<std::unique_ptr<Entity>> entities{};
		} variables{};

	  private:
		sf::Texture large_animator_textures{};
		sf::Texture large_animator_thumbs{};
		sf::Texture small_animator_textures{};
		sf::Texture small_animator_thumbs{};
		sf::Texture enemy_thumbnails{};

		sf::RectangleShape player_box{};
		sf::RectangleShape portalbox{};
		sf::RectangleShape inspbox{};
		sf::RectangleShape platbox{};
		sf::RectangleShape chestbox{};
		sf::RectangleShape savebox{};
		sf::RectangleShape platextent{};
		sf::RectangleShape vinebox{};
		sf::RectangleShape scenerybox{};

		// read and write
		struct {
			dj::Json inspectables{};
		} data{};
	};
} // namespace pi