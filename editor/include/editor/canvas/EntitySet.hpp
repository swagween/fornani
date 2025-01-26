
#pragma once

#include <SFML/Graphics.hpp>
#include <string_view>
#include <djson/json.hpp>
#include <filesystem>
#include <iostream>

namespace data {
class ResourceFinder;
}

namespace pi {

class Canvas;

struct Entity {
	Entity(std::string label, int id = 0, sf::Vector2<uint32_t> position = {}, sf::Vector2<uint32_t> dimensions = {}) : label(label), id(id), position(position), dimensions(dimensions){};
	virtual ~Entity() = default;
	virtual std::unique_ptr<Entity> clone() const;
	virtual void serialize(dj::Json& out);
	virtual void unserialize(dj::Json& in);
	std::string label{};
	sf::Vector2<uint32_t> position{};
	sf::Vector2<uint32_t> dimensions{};
	int id{};
	bool repeatable{};

	bool highlighted{};
	bool selected{};

	// helpers
	void render(sf::RenderWindow& win, sf::Vector2<float> cam, float size);
	sf::RectangleShape drawbox{};
	[[nodiscard]] auto get_label() const -> std::string { return label; }
	[[nodiscard]] auto f_dimensions() const -> sf::Vector2<float> { return sf::Vector2<float>(dimensions); }
};

struct Inspectable : public Entity {
	Inspectable(std::string label) : Entity("inspectables") {}
	Inspectable(bool activate_on_contact, std::string key, std::vector<std::vector<std::string>> suites, std::vector<std::vector<std::string>> responses, int alternates)
		: Entity("inspectables", 0, {}, {1, 1}), activate_on_contact(activate_on_contact), key(key), suites(suites), responses(responses), alternates(alternates) {}
	bool activate_on_contact{};
	std::string key{};
	std::vector<std::vector<std::string>> suites{};
	std::vector<std::vector<std::string>> responses{};
	int alternates{};
	std::unique_ptr<Entity> clone() const override { return std::make_unique<Inspectable>(*this); }
	void serialize(dj::Json& out) override {
		Entity::serialize(out);
		constexpr auto empty_array = R"([])";
		auto const wipe = dj::Json::parse(empty_array);
		out["activate_on_contact"] = dj::Boolean{activate_on_contact};
		out["key"] = key;
		out["alternates"] = alternates;
		for (auto i{0}; i <= alternates; ++i) {
			auto next = std::string{key + std::to_string(i)};
			out[next].push_back(wipe);
			for (auto& suite : suites) {
				auto out_set = wipe;
				for (auto& message : suite) { out_set.push_back(message); }
				out[next]["suite"].push_back(out_set);
			}
			for (auto& response : responses) {
				auto out_set = wipe;
				for (auto& message : response) { out_set.push_back(message); }
				out[next]["responses"].push_back(out_set);
			}
		}
	}
	void unserialize(dj::Json& in) override {
		Entity::unserialize(in);
		constexpr auto empty_array = R"([])";
		auto const wipe = dj::Json::parse(empty_array);
		activate_on_contact = static_cast<bool>(in["activate_on_contact"].as_bool());
		key = in["key"].as_string();
		alternates = in["alternates"].as<int>();
		for (auto i{0}; i <= alternates; ++i) {
			auto next = std::string{key + std::to_string(i)};
			for (auto& in_suite : in[next]["suite"].array_view()) {
				for (auto& message : in_suite.array_view()) { suites.push_back(std::vector<std::string>{message.as_string().data()}); }
			}
			for (auto& in_response : in[next]["responses"].array_view()) {
				for (auto& message : in_response.array_view()) { responses.push_back(std::vector<std::string>{message.as_string().data()}); }
			}
		}
	}
};

struct Critter : public Entity {
	Critter() : Entity("enemies") { repeatable = true; }
	std::unique_ptr<Entity> clone() const override { return std::make_unique<Critter>(*this); }
	void serialize(dj::Json& out) override { Entity::serialize(out); }
	void unserialize(dj::Json& in) override { Entity::unserialize(in); }
};

struct Animator : public Entity {
	Animator() : Entity("animators") { repeatable = true; }
	bool automatic{};
	bool foreground{};
	int style{};
	std::unique_ptr<Entity> clone() const override { return std::make_unique<Animator>(*this); }
	void serialize(dj::Json& out) override { Entity::serialize(out); }
	void unserialize(dj::Json& in) override { Entity::unserialize(in); }
};

struct Portal : public Entity {
	Portal(sf::Vector2u dimensions, bool activate_on_contact, bool already_open, int source_map_id, int destination_map_id, bool locked, int key_id)
		: Entity("portals", 0, {}, dimensions), activate_on_contact(activate_on_contact), already_open(already_open), source_map_id(source_map_id), destination_map_id(destination_map_id), locked(locked), key_id(key_id) {}
	bool activate_on_contact{};
	bool already_open{};
	int source_map_id{};
	int destination_map_id{};
	bool locked{};
	int key_id{};
	std::unique_ptr<Entity> clone() const override { return std::make_unique<Portal>(*this); }
	void serialize(dj::Json& out) override {
		Entity::serialize(out);
		out["activate_on_contact"] = dj::Boolean{activate_on_contact};
		out["already_open"] = dj::Boolean{already_open};
		out["source_map_id"] = source_map_id;
		out["destination_map_id"] = destination_map_id;
		out["locked"] = dj::Boolean{locked};
		out["key_id"] = key_id;
	}
	void unserialize(dj::Json& in) override {
		Entity::unserialize(in);
		activate_on_contact = static_cast<bool>(in["activate_on_contact"].as_bool());
		already_open = static_cast<bool>(in["already_open"].as_bool());
		source_map_id = in["source_map_id"].as<int>();
		destination_map_id = in["destination_map_id"].as<int>();
		locked = static_cast<bool>(in["locked"].as_bool());
		key_id = in["key_id"].as<int>();
	}
};

struct InteractiveScenery : public Entity {
	InteractiveScenery() : Entity("interactive_scenery") { repeatable = true; }
	int length{};
	int size{};
	bool foreground{};
	int type{};
	bool has_platform{};
	std::vector<int> link_indeces{};
	std::unique_ptr<Entity> clone() const override { return std::make_unique<InteractiveScenery>(*this); }
	void serialize(dj::Json& out) override { Entity::serialize(out); }
	void unserialize(dj::Json& in) override { Entity::unserialize(in); }
};

struct NPC : public Entity {
	NPC() : Entity("npcs") {}
	bool background{};
	std::vector<std::vector<std::string>> suites{};
	std::unique_ptr<Entity> clone() const override { return std::make_unique<NPC>(*this); }
	void serialize(dj::Json& out) override { Entity::serialize(out); }
	void unserialize(dj::Json& in) override { Entity::unserialize(in); }
};

struct Platform : public Entity {
	Platform(sf::Vector2u dim, int extent, std::string type, float start) : Entity("platforms", 0, {}, dim), extent(extent), type(type), start(start) { repeatable = true; }
	int extent{};
	std::string type{};
	float start{};
	std::unique_ptr<Entity> clone() const override { return std::make_unique<Platform>(*this); }
	void serialize(dj::Json& out) override {
		Entity::serialize(out);
		out["extent"] = extent;
		out["type"] = type;
		out["start"] = start;
	}
	void unserialize(dj::Json& in) override { Entity::unserialize(in); }
};

struct Chest : public Entity {
	Chest() : Entity("chests") {}
	int item_id{};
	int type{};
	float rarity{};
	int amount{};
	std::unique_ptr<Entity> clone() const override { return std::make_unique<Chest>(*this); }
	void serialize(dj::Json& out) override {
		Entity::serialize(out);
		out["item_id"] = item_id;
		out["type"] = type;
		out["rarity"] = rarity;
		out["amount"] = amount;
	}
	void unserialize(dj::Json& in) override { Entity::unserialize(in); }
};

struct Scenery : public Entity {
	Scenery() : Entity("scenery") { repeatable = true; }
	int style{};
	int layer{};
	int variant{};
	std::unique_ptr<Entity> clone() const override { return std::make_unique<Scenery>(*this); }
	void serialize(dj::Json& out) override { Entity::serialize(out); }
	void unserialize(dj::Json& in) override { Entity::unserialize(in); }
};

struct SwitchBlock : public Entity {
	SwitchBlock() : Entity("switch_blocks") { repeatable = true; }
	int type{};
	std::unique_ptr<Entity> clone() const override { return std::make_unique<SwitchBlock>(*this); }
	void serialize(dj::Json& out) override {
		Entity::serialize(out);
		out["type"] = type;
	}
	void unserialize(dj::Json& in) override { Entity::unserialize(in); }
};

struct SwitchButton : public Entity {
	SwitchButton(int id, int type) : Entity("switches", id), type(type) {}
	int type{};
	std::unique_ptr<Entity> clone() const override { return std::make_unique<SwitchButton>(*this); }
	void serialize(dj::Json& out) override {
		Entity::serialize(out);
		out["type"] = type;
	}
	void unserialize(dj::Json& in) override { Entity::unserialize(in); }
};

struct Destroyer : public Entity {
	Destroyer() : Entity("destroyers") { repeatable = true; }
	std::unique_ptr<Entity> clone() const override { return std::make_unique<Destroyer>(*this); }
	void serialize(dj::Json& out) override { Entity::serialize(out); }
	void unserialize(dj::Json& in) override { Entity::unserialize(in); }
};

struct SavePoint : public Entity {
	SavePoint(int id) : Entity("save_point", id, {}, {1, 1}) {}
	std::unique_ptr<Entity> clone() const override { return std::make_unique<SavePoint>(*this); }
	void serialize(dj::Json& out) override { Entity::serialize(out); }
	void unserialize(dj::Json& in) override { Entity::unserialize(in); }
};

class EntitySet {
  public:
	EntitySet() = default;
	EntitySet(data::ResourceFinder& finder, dj::Json& metadata, std::string const& room_name);
	void render(Canvas& map, sf::RenderWindow& win, sf::Vector2<float> cam);
	void load(data::ResourceFinder& finder, dj::Json& metadata, std::string const& room_name);
	bool save(data::ResourceFinder& finder, dj::Json& metadata, std::string const& room_name);
	void clear();
	bool has_entity_at(sf::Vector2<uint32_t> pos) const;

	struct {
		std::string music{};
		std::optional<std::unique_ptr<Entity>> save_point{};
		sf::Vector2<uint32_t> player_start{};
		std::vector<std::unique_ptr<Entity>> entities{};
	} variables{};

  private:
	sf::Texture enemy_thumbnails{};
	sf::RectangleShape player_box{};

	// read and write
	struct {
		dj::Json inspectables{};
	} data{};
};

} // namespace pi
