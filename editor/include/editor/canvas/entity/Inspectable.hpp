
#pragma once

#include "editor/canvas/Entity.hpp"

namespace pi {

class Inspectable : public Entity {
  public:
	Inspectable();
	Inspectable(bool activate_on_contact, std::string key, std::vector<std::vector<std::string>> suites, std::vector<std::vector<std::string>> responses, int alternates);

	std::unique_ptr<Entity> clone() const override;
	void serialize(dj::Json& out) override;
	void unserialize(dj::Json const& in) override;
	void expose() override;
	void render(sf::RenderWindow& win, sf::Vector2f cam, float size) override;

  private:
	bool activate_on_contact{};
	std::string key{};
	std::vector<std::vector<std::string>> suites{};
	std::vector<std::vector<std::string>> responses{};
	int alternates{};
};

} // namespace pi
