
#pragma once

#include <SFML/Graphics.hpp>
#include <fornani/systems/Notification.hpp>
#include <string_view>
#include <vector>

namespace fornani {

namespace automa {
struct ServiceProvider;
}

class NotificationManager {
  public:
	void push_notification(automa::ServiceProvider& svc, std::string_view message);
	void update(automa::ServiceProvider& svc);
	void render(sf::RenderWindow& win);
	Notification& get_latest() { return m_notifications.back(); }

  private:
	std::vector<Notification> m_notifications{};
};

} // namespace fornani
