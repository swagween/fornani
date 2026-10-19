#pragma once

#include <memory>
#include <string_view>

#include <fornani/entity/SpecialBlock.hpp>

namespace fornani {

class SpecialBlockFactory {
  public:
	[[nodiscard]] static auto create(automa::ServiceProvider& svc, sf::Vector2u dimensions, int id, std::string_view type) -> std::unique_ptr<SpecialBlock>;

	[[nodiscard]] static auto create(automa::ServiceProvider& svc, dj::Json const& in) -> std::unique_ptr<SpecialBlock>;

	[[nodiscard]] static auto create(automa::ServiceProvider& svc, world::Map& map, dj::Json const& in) -> std::unique_ptr<SpecialBlock>;
};

} // namespace fornani
