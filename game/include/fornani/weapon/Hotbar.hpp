
#pragma once

#include <fornani/systems/Register.hpp>
#include <fornani/utils/Circuit.hpp>

namespace fornani::arms {

class Hotbar {
  public:
	explicit Hotbar(int size);
	void add(std::string_view tag);
	void remove(std::string_view tag);
	void switch_weapon(automa::ServiceProvider& svc, int next);
	void set_selection(std::string_view tag);
	void sync();
	bool has(std::string_view tag) const;

	[[nodiscard]] auto get_selection() const -> int { return selection.get(); }
	[[nodiscard]] auto get_tag() const -> std::string_view { return m_tags[selection.get()]; }
	[[nodiscard]] auto get_tag(int index) const -> std::string_view { return m_tags[index]; }
	[[nodiscard]] auto size() const -> std::size_t { return m_tags.size(); }
	[[nodiscard]] auto switched() const -> bool { return selection.get() != previous; }

	// for debug
	[[nodiscard]] auto get_tags() const -> Register<std::string> const& { return m_tags; }

  private:
	util::Circuit selection{1};
	Register<std::string> m_tags{};
	int previous{-1};
};

} // namespace fornani::arms
