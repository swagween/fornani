
#include "QuestTracker.hpp"
#include <iostream>

namespace fornani {

QuestTracker::QuestTracker() {
	suites.standard.quests.insert({20, Quest{20, "bit"}});
	suites.standard.quests.insert({15, Quest{15, "boiler"}});
	suites.inspectables.quests.insert({1, Quest{1, "bryns_notebook"}});
}

int QuestTracker::get_progression(QuestType type, int id) { 
	auto ret{0};
	if (type == QuestType::standard) { ret = suites.standard.get_progression(id); }
	if (type == QuestType::inspectable) { ret = suites.inspectables.get_progression(id); }
	if (type == QuestType::standard) { ret = suites.standard.get_progression(id); }
	return ret; 
}
void QuestTracker::progress(QuestType type, int id, int source, int amount) {
	if (type == QuestType::standard && suites.standard.quests.contains(id)) { suites.standard.quests.at(id).progress(source, amount); }
	if (type == QuestType::inspectable && suites.inspectables.quests.contains(id)) { suites.inspectables.quests.at(id).progress(source, amount); }
}

void QuestTracker::process(util::QuestKey key) { progress(static_cast<QuestType>(key.type), key.id, key.source_id, key.amount); }

} // namespace fornani