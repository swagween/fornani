
#pragma once

#include <memory>

namespace data {
class ResourceFinder;
}

namespace pi {

class Tool;
class Console;

class PopupHandler {
  public:
	void launch(data::ResourceFinder& finder, Console& console, char const* label, std::unique_ptr<Tool>& tool, int room_id = 0);

  private:
	void help_marker(char const* desc);
};

} // namespace pi
