
#pragma once

#include <SFML/Graphics.hpp>
#include "../utils/Counter.hpp"
#include "../utils/QuestCode.hpp"
#include <string_view>
#include <unordered_map>
#include <vector>

namespace fornani {

// needs some further thinking

enum class QuestStatus { not_started, started, complete };
enum class QuestType { null, inspectable, item, standard };

struct Quest {
	int id{};
	std::string_view label{};
	QuestStatus status{QuestStatus::not_started};
	util::Counter progression{};
	std::vector<int> sources{};
	void progress(int source, int amount = 1) {
		// don't progress quests from the same source
		for (auto& src : sources) {
			if (src == source) { return; }
		}
		for (int i = 0; i < amount; ++i) { progression.update(); }
		sources.push_back(source);
	}
};

struct QuestSuite {
	std::unordered_map<int, Quest> quests{};
	int get_progression(int id) { return quests.contains(id) ? quests.at(id).progression.get_count() : 0; };
};

class QuestTracker {
  public:
	QuestTracker();
	int get_progression(QuestType type, int id);
	void progress(QuestType type, int id, int source, int amount = 1);
	void process(util::QuestKey key);

  private:
	struct {
		QuestSuite standard{};
		QuestSuite inspectables{};
	} suites{};
};

} // namespace fornani