
#include "QuestTracker.hpp"
#include <iostream>

namespace fornani {

QuestTracker::QuestTracker() {
	suites.temporaries.quests.insert({4, {4, "player_retry"}});
	suites.npc.quests.insert({20, Quest{20, "bit"}});
	suites.item.quests.insert({12, {12, "bit_cell_key"}});
	suites.item.quests.insert({199, {199, "obtained"}});
	suites.standard.quests.insert({15, Quest{15, "boiler"}});
	suites.inspectables.quests.insert({1, Quest{1, "bryns_notebook"}});
	suites.inspectables.quests.insert({110, Quest{110, "firstwind_lab_computer"}});
	suites.destroyers.quests.insert({110, Quest{110, "firstwind_lab_destroyers"}});
}

int QuestTracker::get_progression(QuestType type, int id) { 
	auto ret{0};
	if (type == QuestType::npc) { ret = suites.npc.get_progression(id); }
	if (type == QuestType::item) { ret = suites.item.get_progression(id); }
	if (type == QuestType::inspectable) { ret = suites.inspectables.get_progression(id); }
	if (type == QuestType::standard) { ret = suites.standard.get_progression(id); }
	if (type == QuestType::destroyers) { ret = suites.destroyers.get_progression(id); }
	return ret; 
}
void QuestTracker::progress(QuestType type, int id, int source, int amount, bool hard_set) {
	if (type == QuestType::npc && suites.npc.quests.contains(id)) { suites.npc.quests.at(id).progress(source, amount, hard_set); }
	if (type == QuestType::item && suites.item.quests.contains(id)) { suites.item.quests.at(id).progress(source, amount, hard_set); }
	if (type == QuestType::standard && suites.standard.quests.contains(id)) { suites.standard.quests.at(id).progress(source, amount, hard_set); }
	if (type == QuestType::inspectable && suites.inspectables.quests.contains(id)) { suites.inspectables.quests.at(id).progress(source, amount, hard_set); }
	if (type == QuestType::destroyers && suites.destroyers.quests.contains(id)) { suites.destroyers.quests.at(id).progress(source, amount, hard_set); }
}

void QuestTracker::process(util::QuestKey key) {
	if (key.hard_set > 0) {
		while (get_progression(static_cast<QuestType>(key.type), key.id) < key.hard_set) {
			progress(static_cast<QuestType>(key.type), key.id, key.source_id, key.amount, true);
			std::cout << get_progression(static_cast<QuestType>(key.type), key.id) << "\n";
		}
	} else {
		progress(static_cast<QuestType>(key.type), key.id, key.source_id, key.amount);
	}
}


} // namespace fornani