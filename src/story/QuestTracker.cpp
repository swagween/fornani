
#include "QuestTracker.hpp"
#include <iostream>

namespace fornani {

QuestTracker::QuestTracker() { suites.npc.quests.insert({20, Quest{20, "bit"}}); }

int QuestTracker::get_progression(QuestType type, int id) { 
	auto ret{0};
	if (type == QuestType::npc) { ret = suites.npc.get_progression(id); }
	return ret; 
}
void QuestTracker::progress(QuestType type, int id) {
	if (type == QuestType::npc && suites.npc.quests.contains(id)) {
		suites.npc.quests.at(id).progress();
	}
}
} // namespace fornani