
#pragma once

#include <string>
#include "fornani/particle/Sparkler.hpp"
#include "fornani/utils/RectPath.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::gui {

enum class ItemWidgetType { gun, item };

class ItemWidget {
  public:
	explicit ItemWidget(automa::ServiceProvider& svc, ItemWidgetType type, int id);
	void update(automa::ServiceProvider& svc);
	void render(automa::ServiceProvider& svc, sf::RenderWindow& win);
	void bring_in();
	void send_out();
	void remove_sparkler();

  private:
	int m_id;
	ItemWidgetType m_type;
	struct {
		sf::Sprite sticker;
		sf::Sprite item;
	} m_sprites;
	util::RectPath m_path;
	std::optional<vfx::Sparkler> m_sparkler;
};

} // namespace fornani::gui
