
#pragma once

#include <memory>

namespace pi {
class Tool;
class Console;
class ResourceFinder;
class PopupHandler{
  public:
	void launch(ResourceFinder& finder, Console& console, char const* label, std::unique_ptr<Tool>& tool);

  private:
	void help_marker(char const* desc);
};
} // namespace pi