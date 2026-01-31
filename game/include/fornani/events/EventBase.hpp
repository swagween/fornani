
#pragma once

#include <fornani/events/Subscription.hpp>
#include <fornani/utils/Polymorphic.hpp>
#include <functional>
#include <utility>
#include <vector>

namespace fornani {

struct IEvent : public Polymorphic {};

template <typename... Args>
class EventBase : public IEvent {
  public:
	using Callback = std::function<void(Args...)>;
	using SubscriptionID = std::size_t;

	Subscription subscribe(Callback cb) {
		auto id = ++next_id;
		m_callbacks.emplace_back(id, std::move(cb));
		return Subscription{[this, id]() { unsubscribe(id); }};
	}

	void dispatch(Args... args) {
		for (auto& [_, cb] : m_callbacks) { cb(args...); }
	}

	void clear() { m_callbacks.clear(); }

  private:
	void unsubscribe(SubscriptionID id) {
		std::erase_if(m_callbacks, [&](auto const& pair) { return pair.first == id; });
	}

	std::vector<std::pair<SubscriptionID, Callback>> m_callbacks;
	SubscriptionID next_id{0};
};

} // namespace fornani
