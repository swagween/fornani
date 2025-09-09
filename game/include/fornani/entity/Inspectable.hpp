
#pragma once

#include <fornani/entity/Entity.hpp>
#include <fornani/gui/console/Message.hpp>

namespace fornani {

class Inspectable : public Entity {
  public:
	Inspectable(automa::ServiceProvider& svc, dj::Json const& in);
	Inspectable(automa::ServiceProvider& svc, std::vector<std::vector<gui::BasicMessage>> suite, std::vector<std::vector<gui::BasicMessage>> responses, bool activate_on_contact, std::string key, int alternates, bool instant);

	std::unique_ptr<Entity> clone() const override;
	void serialize(dj::Json& out) override;
	void unserialize(dj::Json const& in) override;
	void expose() override;
	void render(sf::RenderWindow& win, sf::Vector2f cam, float size) override;

  private:
	bool m_activate_on_contact{};
	bool m_instant{};
	std::string m_key{};
	std::vector<std::vector<gui::BasicMessage>> m_suites{};
	std::vector<std::vector<gui::BasicMessage>> m_responses{};
	int m_alternates{};
};

} // namespace fornani
