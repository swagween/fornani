
#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <utility>
#include <vector>

namespace fornani {

struct Slot {};

struct Subscription {
	std::function<void()> unsubscribe;

	Subscription() = default;
	explicit Subscription(std::function<void()> fn) : unsubscribe(std::move(fn)) {}

	Subscription(Subscription&&) = default;
	Subscription& operator=(Subscription&&) = default;

	Subscription(Subscription const&) = delete;
	Subscription& operator=(Subscription const&) = delete;

	~Subscription() {
		if (unsubscribe) { unsubscribe(); }
	}
};

class SubscriptionGroup {
  public:
	void add(Subscription sub) { m_subs.push_back(std::move(sub)); }

	void clear() { m_subs.clear(); }

  private:
	std::vector<Subscription> m_subs;
};

} // namespace fornani
