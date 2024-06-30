
#include "QuestTracker.hpp"
#include <iostream>

namespace fornani {
QuestTracker::QuestTracker() { npc_quests.quests.push_back({Quest{20, "bit"}}); }
int QuestTracker::get_progression(QuestType type, int id) { 
	auto ret{0};
	if (type == QuestType::npc) { npc_quests.get_progression(id); }
	return ret; 
}
} // namespace fornani