
#pragma once

#include <SFML/Graphics.hpp>
#include <djson/json.hpp>
#include <fornani/entity/Entity.hpp>
#include <fornani/graphics/SpriteBatch.hpp>
#include <fornani/io/Logger.hpp>
#include <filesystem>
#include <memory>
#include <unordered_map>

namespace fornani::world {
class Map;
}

namespace fornani {
class ResourceFinder;

template <typename T>
std::unique_ptr<Entity> create_entity(automa::ServiceProvider& svc, dj::Json const& in) {
	return std::make_unique<T>(svc, in);
}

template <typename T>
std::unique_ptr<Entity> create_registered_entity(automa::ServiceProvider& svc, world::Map& map, dj::Json const& in) {
	return std::make_unique<T>(svc, map, in);
}

using CreateEntitySignature = decltype(&create_entity<Entity>);
using CreateRegisteredEntitySignature = decltype(&create_registered_entity<Entity>);

class EntitySet {
  public:
	EntitySet() = default;
	EntitySet(automa::ServiceProvider& svc, ResourceFinder& finder, dj::Json& metadata, std::string const& room_name, bool delegated = false);
	EntitySet(automa::ServiceProvider& svc, world::Map& map, ResourceFinder& finder, dj::Json& metadata, std::string const& room_name);
	void render(sf::RenderWindow& win, sf::Vector2f cam, sf::Vector2f origin, float cell_size);
	void load(automa::ServiceProvider& svc, ResourceFinder& finder, dj::Json& metadata, std::string const& room_name);
	void load_and_register(automa::ServiceProvider& svc, world::Map& map, ResourceFinder& finder, dj::Json& metadata, std::string const& room_name);
	bool save(ResourceFinder& finder, dj::Json& metadata, std::string const& room_name);
	void clear();
	bool has_entity_at(sf::Vector2u pos, bool highlighted_only = false) const;
	bool has_entity_at(sf::Vector2i pos, bool highlighted_only = false) const;
	bool overlaps(Entity& other) const;

	struct {
		sf::Vector2<std::uint32_t> player_start{};
		sf::Vector2<std::uint32_t> player_hot_start{};
		std::vector<std::unique_ptr<Entity>> entities{};
	} variables{};

  private:
	EntityHandle next_handle{};
	SpriteBatch m_batch{};
	sf::RectangleShape player_box{};
	std::unordered_map<std::string, CreateEntitySignature> create_map;
	std::unordered_map<std::string, CreateRegisteredEntitySignature> registered_map;
	io::Logger m_logger{"Pioneer"};
};

} // namespace fornani
