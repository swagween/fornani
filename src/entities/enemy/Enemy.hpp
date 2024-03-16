#pragma once

#include "../Entity.hpp"
#include "../../utils/Collider.hpp"
#include "../../utils/BitFlags.hpp"
#include "../behavior/Animation.hpp"
#include "../../utils/StateFunction.hpp"
#include <string_view>

namespace enemy {

	enum class GeneralFlags {mobile, gravity};

class Enemy : public entity::Entity {
  public:
	Enemy(automa::ServiceProvider& svc, std::string_view label);
	void update(automa::ServiceProvider& svc);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win, sf::Vector2<float> cam);

	virtual void unique_update(){};

  private:
	util::BitFlags<GeneralFlags> general_flags{};
	shape::Collider collider{};
	std::string_view label{};

	struct {
	} metadata{};

	struct {
	} physical{};

	struct {
	} attributes{};

	struct {
	} animation{};
};

} // namespace enemy