
#pragma once

#include <fornani/entity/Entity.hpp>

namespace fornani {

class Inspectable : public Entity {
  public:
	Inspectable(automa::ServiceProvider& svc, dj::Json const& in);
	Inspectable(automa::ServiceProvider& svc, bool activate_on_contact, std::string key, std::vector<std::vector<std::string>> suites, std::vector<std::vector<std::string>> responses, int alternates, bool instant);

	std::unique_ptr<Entity> clone() const override;
	void serialize(dj::Json& out) override;
	void unserialize(dj::Json const& in) override;
	void expose() override;
	void render(sf::RenderWindow& win, sf::Vector2f cam, float size) override;

  private:
	bool m_activate_on_contact{};
	bool m_instant{};
	std::string m_key{};
	std::vector<std::vector<std::string>> m_suites{};
	std::vector<std::vector<std::string>> m_responses{};
	std::vector<std::vector<int>> m_codes{};
	int m_alternates{};
};

} // namespace fornani
