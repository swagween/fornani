
#pragma once

#include "editor/canvas/Entity.hpp"

#include <SFML/Graphics.hpp>
#include <djson/json.hpp>
#include <imgui.h>

#include <string_view>
#include <filesystem>

namespace fornani::data {
class ResourceFinder;
}

namespace pi {

class Canvas;

//struct Animator : public Entity {
//	Animator() : Entity("animators") { repeatable = true; }
//	bool automatic{};
//	bool foreground{};
//	int style{};
//	std::unique_ptr<Entity> clone() const override { return std::make_unique<Animator>(*this); }
//	void serialize(dj::Json& out) override { Entity::serialize(out); }
//	void unserialize(dj::Json& in) override { Entity::unserialize(in); }
//	void expose() override { Entity::expose(); }
//};
//

//
//struct InteractiveScenery : public Entity {
//	InteractiveScenery() : Entity("interactive_scenery") { repeatable = true; }
//	int length{};
//	int size{};
//	bool foreground{};
//	int type{};
//	bool has_platform{};
//	std::vector<int> link_indeces{};
//	std::unique_ptr<Entity> clone() const override { return std::make_unique<InteractiveScenery>(*this); }
//	void serialize(dj::Json& out) override { Entity::serialize(out); }
//	void unserialize(dj::Json& in) override { Entity::unserialize(in); }
//	void expose() override { Entity::expose(); }
//};
//
//struct NPC : public Entity {
//	NPC() : Entity("npcs") {}
//	bool background{};
//	std::vector<std::vector<std::string>> suites{};
//	std::unique_ptr<Entity> clone() const override { return std::make_unique<NPC>(*this); }
//	void serialize(dj::Json& out) override { Entity::serialize(out); }
//	void unserialize(dj::Json& in) override { Entity::unserialize(in); }
//	void expose() override { Entity::expose(); }
//};

//struct Chest : public Entity {
//	Chest() : Entity("chests") {}
//	int item_id{};
//	int type{};
//	float rarity{};
//	int amount{};
//	std::unique_ptr<Entity> clone() const override { return std::make_unique<Chest>(*this); }
//	void serialize(dj::Json& out) override {
//		Entity::serialize(out);
//		out["item_id"] = item_id;
//		out["type"] = type;
//		out["rarity"] = rarity;
//		out["amount"] = amount;
//	}
//	void unserialize(dj::Json& in) override { Entity::unserialize(in); }
//	void expose() override { Entity::expose(); }
//};
//
//struct Scenery : public Entity {
//	Scenery() : Entity("scenery") { repeatable = true; }
//	int style{};
//	int layer{};
//	int variant{};
//	std::unique_ptr<Entity> clone() const override { return std::make_unique<Scenery>(*this); }
//	void serialize(dj::Json& out) override { Entity::serialize(out); }
//	void unserialize(dj::Json& in) override { Entity::unserialize(in); }
//	void expose() override { Entity::expose(); }
//};
//
//struct SwitchBlock : public Entity {
//	SwitchBlock() : Entity("switch_blocks") { repeatable = true; }
//	int type{};
//	std::unique_ptr<Entity> clone() const override { return std::make_unique<SwitchBlock>(*this); }
//	void serialize(dj::Json& out) override {
//		Entity::serialize(out);
//		out["type"] = type;
//	}
//	void unserialize(dj::Json& in) override { Entity::unserialize(in); }
//	void expose() override { Entity::expose(); }
//};
//
//struct SwitchButton : public Entity {
//	SwitchButton(int id, int type) : Entity("switches", id), type(type) {}
//	int type{};
//	std::unique_ptr<Entity> clone() const override { return std::make_unique<SwitchButton>(*this); }
//	void serialize(dj::Json& out) override {
//		Entity::serialize(out);
//		out["type"] = type;
//	}
//	void unserialize(dj::Json& in) override { Entity::unserialize(in); }
//	void expose() override { Entity::expose(); }
//};
//
//struct Destroyer : public Entity {
//	Destroyer() : Entity("destroyers") { repeatable = true; }
//	std::unique_ptr<Entity> clone() const override { return std::make_unique<Destroyer>(*this); }
//	void serialize(dj::Json& out) override { Entity::serialize(out); }
//	void unserialize(dj::Json& in) override { Entity::unserialize(in); }
//	void expose() override { Entity::expose(); }
//};

class EntitySet {
  public:
	EntitySet() = default;
	EntitySet(fornani::data::ResourceFinder& finder, dj::Json& metadata, std::string const& room_name);
	void render(Canvas& map, sf::RenderWindow& win, sf::Vector2<float> cam);
	void load(fornani::data::ResourceFinder& finder, dj::Json& metadata, std::string const& room_name);
	bool save(fornani::data::ResourceFinder& finder, dj::Json& metadata, std::string const& room_name);
	void clear();
	bool has_entity_at(sf::Vector2<uint32_t> pos, bool highlighted_only = false) const;
	bool overlaps(Entity& other) const;

	struct {
		sf::Vector2<uint32_t> player_start{};
		sf::Vector2<uint32_t> player_hot_start{};
		std::vector<std::unique_ptr<Entity>> entities{};
	} variables{};

  private:
	sf::RectangleShape player_box{};
	struct {
		dj::Json inspectables{};
	} data{};
};

} // namespace pi
