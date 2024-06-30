
#pragma once

#include <SFML/Graphics.hpp>
#include "../utils/Counter.hpp"
#include <string_view>
#include <unordered_map>
#include <vector>

namespace fornani {

// needs some further thinking

enum class QuestStatus { not_started, started, complete };
enum class QuestType { null, inspectable, item, npc };

struct Quest {
	int id{};
	std::string_view label{};
	QuestStatus status{QuestStatus::not_started};
	util::Counter progression{};
	void progress() { progression.update(); }
};

struct QuestSuite {
	std::vector<Quest> quests{};
	int get_progression(int id) { return quests.at(id).progression.get_count(); };
};

class QuestTracker {
  public:
	QuestTracker();
	int get_progression(QuestType type, int id);
	void progress(QuestType type, int id);

  private:
	QuestSuite npc_quests{};
};

} // namespace fornani