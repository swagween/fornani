
#pragma once

#include "editor/canvas/Entity.hpp"

namespace pi {

class Inspectable : public Entity {
  public:
	Inspectable(std::string label);
	Inspectable(bool activate_on_contact, std::string key, std::vector<std::vector<std::string>> suites, std::vector<std::vector<std::string>> responses, int alternates);

	std::unique_ptr<Entity> clone() const override;
	void serialize(dj::Json& out) override;
	void unserialize(dj::Json& in) override;
	void expose() override {
		Entity::expose();
		ImGui::Checkbox("Activate on Contact", &activate_on_contact);
		ImGui::InputInt("Alternates", &alternates);
	}
	void render(sf::RenderWindow& win, sf::Vector2<float> cam) override;

  private:
	bool activate_on_contact{};
	std::string key{};
	std::vector<std::vector<std::string>> suites{};
	std::vector<std::vector<std::string>> responses{};
	int alternates{};

};

} // namespace pi
