
#pragma once

#include <SFML/Graphics.hpp>
#include "../utils/Counter.hpp"
#include <string_view>
#include <unordered_map>
#include <vector>

namespace fornani {

// needs some further thinking

enum class Status { not_started, started, complete };

struct Quest {
	int id{};
	std::string_view label{};
	Status status{Status::not_started};
	util::Counter progression{};
	void progress() { progression.update(); }
};

struct QuestSuite {};

class QuestTracker {
  public:
  private:
};

} // namespace fornani