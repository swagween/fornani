
#pragma once

#include <fornani/utils/Polymorphic.hpp>
#include <functional>
#include <string>

namespace fornani {

class IEvent {
  public:
	virtual ~IEvent() {};
	virtual std::string const& get_label() const = 0;
};

template <typename... _args>
class Event : public IEvent {
  public:
	using callback = std::function<void(_args...)>;
	~Event() {}

	explicit Event(std::string const& label, callback const& cb) : m_label(label), m_callback_function(cb) {}

	std::string const& get_label() const override { return this->m_label; }

	void trigger(_args... a) { this->m_callback_function(a...); }

  private:
	std::string m_label;
	callback const m_callback_function;
};

} // namespace fornani
