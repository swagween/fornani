
#pragma once

#include <djson/json.hpp>
#include <fornani/entity/Entity.hpp>
#include <fornani/story/Quest.hpp>
#include <memory>

namespace fornani {

class NPC : public Entity {
  public:
	NPC(automa::ServiceProvider& svc, dj::Json const& in);
	NPC(automa::ServiceProvider& svc, int id, std::string_view label, std::vector<std::vector<int>> const suites);
	std::unique_ptr<Entity> clone() const override;
	void serialize(dj::Json& out) override;
	void unserialize(dj::Json const& in) override;
	void expose() override;
	void render(sf::RenderWindow& win, sf::Vector2f cam, float size) override;

  private:
	std::string m_label{};
	std::vector<std::vector<int>> m_suites{};
	std::vector<QuestContingency> m_contingencies{};

	bool m_background{};
	bool m_hidden{};
};

} // namespace fornani
