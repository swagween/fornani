
#pragma once

#include "editor/canvas/Entity.hpp"
#include "fornani/io/Logger.hpp"

#include <imgui.h>
#include <SFML/Graphics.hpp>
#include <djson/json.hpp>

#include <filesystem>
#include <memory>
#include <string_view>
#include <unordered_map>
#include "editor/canvas/entity/Bed.hpp"
#include "editor/canvas/entity/Chest.hpp"
#include "editor/canvas/entity/Destructible.hpp"
#include "editor/canvas/entity/Enemy.hpp"
#include "editor/canvas/entity/Inspectable.hpp"
#include "editor/canvas/entity/Platform.hpp"
#include "editor/canvas/entity/Portal.hpp"
#include "editor/canvas/entity/SavePoint.hpp"
#include "editor/canvas/entity/SwitchBlock.hpp"
#include "editor/canvas/entity/SwitchButton.hpp"

namespace fornani::data {
class ResourceFinder;
}

namespace pi {

class Canvas;

template <typename T>
std::unique_ptr<Entity> create_entity(fornani::automa::ServiceProvider& svc, dj::Json const& in) {
	return std::make_unique<T>(svc, in);
}

using CreateEntitySignature = decltype(&create_entity<Entity>);

class EntitySet {
  public:
	EntitySet() = default;
	EntitySet(fornani::automa::ServiceProvider& svc, fornani::data::ResourceFinder& finder, dj::Json& metadata, std::string const& room_name);
	void render(Canvas& map, sf::RenderWindow& win, sf::Vector2f cam);
	void load(fornani::automa::ServiceProvider& svc, fornani::data::ResourceFinder& finder, dj::Json& metadata, std::string const& room_name);
	bool save(fornani::data::ResourceFinder& finder, dj::Json& metadata, std::string const& room_name);
	void clear();
	bool has_entity_at(sf::Vector2<std::uint32_t> pos, bool highlighted_only = false) const;
	bool overlaps(Entity& other) const;

	struct {
		sf::Vector2<std::uint32_t> player_start{};
		sf::Vector2<std::uint32_t> player_hot_start{};
		std::vector<std::unique_ptr<Entity>> entities{};
	} variables{};

  private:
	sf::RectangleShape player_box{};
	std::unordered_map<std::string, CreateEntitySignature> create_map;
	fornani::io::Logger m_logger{"Pioneer"};
};

} // namespace pi
