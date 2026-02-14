
#include <fornani/systems/NotificationManager.hpp>
#include <ranges>

namespace fornani {

void NotificationManager::push_notification(automa::ServiceProvider& svc, std::string_view message) { m_notifications.push_back(Notification{svc, message}); }

void NotificationManager::update(automa::ServiceProvider& svc) {
	for (auto& notif : m_notifications) { notif.update(svc); }
	std::erase_if(m_notifications, [](auto const& n) { return n.is_complete(); });
}

void NotificationManager::render(sf::RenderWindow& win) {
	for (auto [i, notif] : std::views::enumerate(m_notifications)) { notif.render(win, static_cast<float>(m_notifications.size() - i - 1) * 32.f); }
}

} // namespace fornani
