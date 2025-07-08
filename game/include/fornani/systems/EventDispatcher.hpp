
#pragma once

#include <fornani/gui/Console.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/systems/Event.hpp>
#include <map>
#include <memory>

namespace fornani {

class EventDispatcher {
  public:
	EventDispatcher() = default;
	~EventDispatcher() {
		for (auto el : m_event_list) {
			for (auto e : el.second) delete e;
		}
	}

	void register_event(IEvent* event) {
		if (event) {
			m_event_list[event->get_label()].push_back(event);
			NANI_LOG_DEBUG(m_logger, "Registered event <{}>", event->get_label());
		}
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
			if (Event<_args...>* event = dynamic_cast<Event<_args...>*>(ie)) {
				event->trigger(a...);
				NANI_LOG_DEBUG(m_logger, "Dispatched event <{}>", event_name);
			} else {
				NANI_LOG_DEBUG(m_logger, "Failed to dynamically cast event <{}>", event_name);
			}
		}
	}

  private:
	std::map<std::string, std::vector<IEvent*>> m_event_list;

	io::Logger m_logger{"System"};
};

} // namespace fornani
