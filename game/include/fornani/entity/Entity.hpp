
#pragma once

#include <SFML/Graphics.hpp>
#include <djson/json.hpp>
#include <fornani/core/Fwd.hpp>
#include <fornani/graphics/Animatable.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/story/Quest.hpp>
#include <fornani/utils/Constants.hpp>
#include <fornani/utils/ID.hpp>
#include <fornani/utils/IWorldPositionable.hpp>

namespace fornani {

using EntityHandle = std::uint64_t;

enum class EntityFlags { spawn_denied };

class Entity : public virtual Animatable, public IWorldPositionable {
  public:
	explicit Entity(automa::ServiceProvider& svc, dj::Json const& in, std::string_view label, sf::Vector2i dim = constants::i_cell_vec);
	explicit Entity(automa::ServiceProvider& svc, std::string_view label, int to_id, sf::Vector2<std::uint32_t> dim = {1, 1});

	virtual std::unique_ptr<Entity> clone() const;
	virtual void serialize(dj::Json& out);
	virtual void unserialize(dj::Json const& in);
	virtual void expose();
	virtual void update([[maybe_unused]] automa::ServiceProvider& svc, [[maybe_unused]] world::Map& map, [[maybe_unused]] SceneContext& context, [[maybe_unused]] player::Player& player);
	virtual void render(sf::RenderWindow& win, sf::Vector2f cam, float size);
	virtual void submit(Renderer& renderer);
	void set_handle(EntityHandle to) { m_handle = to; }
	void set_position(sf::Vector2u to_position);
	void set_stable_id(int room_id) { p_stable_id = StableID::from(room_id, get_grid_position().x, get_grid_position().y); }
	bool repeatable{};
	bool overwrite{};
	bool unique{};

	bool highlighted{};
	bool copyable{true};
	bool selected{};
	bool moved{};
	bool stackable{};
	bool batch{};

	// helpers
	sf::RectangleShape drawbox{};
	[[nodiscard]] auto get_handle() const -> EntityHandle { return m_handle; }
	[[nodiscard]] auto get_id() const -> int { return m_id; }
	[[nodiscard]] auto get_stable_id() const -> StableID::underlying_type { return p_stable_id.get(); }
	[[nodiscard]] auto get_label() const -> std::string { return m_label; }
	[[nodiscard]] auto contains_point(sf::Vector2u test) const -> bool;
	[[nodiscard]] auto spawn_denied() const -> bool { return p_flags.test(EntityFlags::spawn_denied); };

  protected:
	bool m_editor{};
	bool m_textured{true};
	io::Logger m_logger{"Pioneer"};
	StableID p_stable_id{};
	std::optional<QuestContingencySet> p_contingencies{};
	util::BitFlags<EntityFlags> p_flags{};

  private:
	int m_id{};
	std::string m_label{};
	EntityHandle m_handle{};
};

} // namespace fornani
