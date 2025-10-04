
#pragma once

#include <string_view>
#include <unordered_map>
#include <vector>
#include "fornani/utils/Counter.hpp"
#include "fornani/utils/QuestCode.hpp"

namespace fornani::automa {
struct ServiceProvider;
}

namespace fornani::quest {

// needs some further thinking

enum class QuestStatus : std::uint8_t { not_started, started, complete };
enum class QuestType : std::uint8_t { null, inspectable, item, npc, standard, destructibles, time_trials, fetch_text, cutscene, hidden_npcs }; // don't reorder these

struct Quest {
	int id{};
	std::string_view label{};
	QuestStatus status{QuestStatus::not_started};
	util::Counter progression{};
	std::vector<int> sources{};
	void progress(int const source, int const amount = 1, bool const hard_set = false) {
		// don't progress quests from the same source
		for (auto const& src : sources) {
			if (src == source && !hard_set) { return; }
		}
		for (int i = 0; i < amount; ++i) { progression.update(); }
		sources.push_back(source);
	}
};

struct QuestSuite {
	std::unordered_map<int, Quest> quests{};
	int get_progression(int const id) const { return quests.contains(id) ? quests.at(id).progression.get_count() : 0; };
	void reset(int const id) {
		if (quests.contains(id)) { quests.at(id).progression.start(); }
	};
};

class QuestTracker {
  public:
	QuestTracker();
	int get_progression(QuestType type, int id);
	void progress(QuestType type, int id, int source, int amount = 1, bool hard_set = false);
	void reset(QuestType type, int id);
	void process(automa::ServiceProvider& svc, util::QuestKey key);

  private:
	struct {
		QuestSuite standard{};
		QuestSuite temporaries{};
		QuestSuite fetch_text{};
		QuestSuite inspectables{};
		QuestSuite npc{};
		QuestSuite item{};
		QuestSuite destructibles{};
		QuestSuite time_trials{};
		QuestSuite cutscene{};
		QuestSuite hidden_npcs{};
	} suites{};
};

} // namespace fornani::quest
