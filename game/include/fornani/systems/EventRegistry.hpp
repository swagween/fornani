
#pragma once

#include <fornani/events/EventBase.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/io/Logger.hpp>
#include <concepts>
#include <map>
#include <memory>
#include <typeindex>
#include <unordered_map>

namespace fornani {

struct EventRegistry {
	EventRegistry() = default;
	~EventRegistry() = default;

	EventRegistry(EventRegistry const&) = delete;
	EventRegistry& operator=(EventRegistry const&) = delete;

	EventRegistry(EventRegistry&&) noexcept = default;
	EventRegistry& operator=(EventRegistry&&) noexcept = default;

	template <std::derived_from<IEvent> EventType>
	void add(std::unique_ptr<EventType> event) {
		m_events[typeid(EventType)] = std::move(event);
	}

	template <std::derived_from<IEvent> EventType>
	EventType* get_event() const {
		auto it = m_events.find(typeid(EventType));
		if (it == m_events.end()) return nullptr;
		return static_cast<EventType*>(it->second.get());
	}

	template <std::derived_from<IEvent> EventType>
	EventType& get_or_add() {
		auto it = m_events.find(typeid(EventType));
		if (it != m_events.end()) { return *static_cast<EventType*>(it->second.get()); }

		auto event = std::make_unique<EventType>();
		EventType& ref = *event;
		m_events[typeid(EventType)] = std::move(event);
		return ref;
	}

  private:
	std::unordered_map<std::type_index, std::unique_ptr<IEvent>> m_events;
	io::Logger m_logger{"System"};
};

} // namespace fornani
