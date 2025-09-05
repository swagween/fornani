
#pragma once

#include <fornani/gui/console/Console.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/systems/Event.hpp>
#include <map>
#include <memory>

namespace fornani {

class EventDispatcher {
  public:
	EventDispatcher() = default;

	void register_event(std::unique_ptr<IEvent> event) {
		if (event) { m_event_list[event->get_label()].push_back(std::move(event)); }
	}

	template <typename... _args>
	void dispatch_event(std::string const& event_name, _args... a) {
		auto it_event_list = m_event_list.find(event_name);
		NANI_LOG_DEBUG(m_logger, "Trying to dispatch event <{}>", event_name);
		if (it_event_list == m_event_list.end()) {
			NANI_LOG_DEBUG(m_logger, "Failed to find event <{}>", event_name);
			return;
		}
		for (auto& ie : it_event_list->second) {
			if (Event<_args...>* event = dynamic_cast<Event<_args...>*>(ie.get())) {
				event->trigger(a...);
				NANI_LOG_DEBUG(m_logger, "Dispatched event <{}>", event_name);
			} else {
				NANI_LOG_DEBUG(m_logger, "Failed to dynamically cast event <{}>", event_name);
			}
		}
	}

  private:
	std::map<std::string, std::vector<std::unique_ptr<IEvent>>> m_event_list;

	io::Logger m_logger{"System"};
};

} // namespace fornani
