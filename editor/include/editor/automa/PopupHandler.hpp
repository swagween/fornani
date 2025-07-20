
#pragma once

#include <memory>

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::data {
class ResourceFinder;
}

namespace pi {

class Tool;
class Console;

class PopupHandler {
  public:
	void launch(fornani::automa::ServiceProvider& svc, fornani::data::ResourceFinder& finder, Console& console, char const* label, std::unique_ptr<Tool>& tool, int room_id = 0);
	[[nodiscard]] auto is_open() const -> bool { return m_is_open; }

  private:
	void help_marker(char const* desc);
	bool m_is_open{};
};

} // namespace pi
