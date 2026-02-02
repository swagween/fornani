
#include "fornani/story/QuestTracker.hpp"
#include "fornani/service/ServiceProvider.hpp"

namespace fornani::quest {

QuestTracker::QuestTracker() {
	suites.temporaries.quests.insert({4, {4, "player_retry"}});
	suites.temporaries.quests.insert({88, {88, "bed"}});
	suites.temporaries.quests.insert({89, {89, "menu_return"}});
	suites.time_trials.quests.insert({59, {59, "bryns_gun"}});
	suites.fetch_text.quests.insert({59, {59, "bg_timer"}});
	suites.npc.quests.insert({20, Quest{20, "bit"}});
	suites.npc.quests.insert({24, Quest{24, "justin"}});
	suites.npc.quests.insert({3, Quest{3, "gobe"}});
	suites.hidden_npcs.quests.insert({24, Quest{24, "justin"}});
	suites.item.quests.insert({12, {12, "bit_cell_key"}});
	suites.item.quests.insert({199, {199, "obtained"}});
	suites.standard.quests.insert({15, Quest{15, "boiler"}});
	suites.inspectables.quests.insert({1, Quest{1, "bryns_notebook"}});
	suites.inspectables.quests.insert({110, Quest{110, "firstwind_lab_computer"}});
	suites.inspectables.quests.insert({111, Quest{111, "firstwind_lab_timer"}});
	suites.destructibles.quests.insert({110, Quest{110, "firstwind_lab_destructibles"}});
	suites.destructibles.quests.insert({115, Quest{115, "firstwind_deck_destructibles"}});
	suites.destructibles.quests.insert({122, Quest{122, "firstwind_atrium_destructibles"}});
	suites.cutscene.quests.insert({3002, Quest{3002, "junkyard_test"}});
	suites.cutscene.quests.insert({6001, Quest{6001, "lady_numbus_1"}});
}

int QuestTracker::get_progression(QuestType type, int id) {
	auto ret{0};
	if (type == QuestType::npc) { ret = suites.npc.get_progression(id); }
	if (type == QuestType::item) { ret = suites.item.get_progression(id); }
	if (type == QuestType::inspectable) { ret = suites.inspectables.get_progression(id); }
	if (type == QuestType::standard) { ret = suites.standard.get_progression(id); }
	if (type == QuestType::destructibles) { ret = suites.destructibles.get_progression(id); }
	if (type == QuestType::time_trials) { ret = suites.time_trials.get_progression(id); }
	if (type == QuestType::fetch_text) { ret = suites.fetch_text.get_progression(id); }
	if (type == QuestType::cutscene) { ret = suites.cutscene.get_progression(id); }
	if (type == QuestType::hidden_npcs) { ret = suites.hidden_npcs.get_progression(id); }
	return ret;
}
void QuestTracker::progress(QuestType type, int id, int source, int amount, bool hard_set) {
	if (type == QuestType::npc && suites.npc.quests.contains(id)) { suites.npc.quests.at(id).progress(source, amount, hard_set); }
	if (type == QuestType::item && suites.item.quests.contains(id)) { suites.item.quests.at(id).progress(source, amount, hard_set); }
	if (type == QuestType::standard && suites.standard.quests.contains(id)) { suites.standard.quests.at(id).progress(source, amount, hard_set); }
	if (type == QuestType::inspectable && suites.inspectables.quests.contains(id)) { suites.inspectables.quests.at(id).progress(source, amount, hard_set); }
	if (type == QuestType::destructibles && suites.destructibles.quests.contains(id)) { suites.destructibles.quests.at(id).progress(source, amount, hard_set); }
	if (type == QuestType::time_trials && suites.time_trials.quests.contains(id)) { suites.time_trials.quests.at(id).progress(source, amount, hard_set); }
	if (type == QuestType::fetch_text && suites.fetch_text.quests.contains(id)) { suites.fetch_text.quests.at(id).progress(source, amount, hard_set); }
	if (type == QuestType::cutscene && suites.cutscene.quests.contains(id)) { suites.cutscene.quests.at(id).progress(source, amount, hard_set); }
	if (type == QuestType::hidden_npcs && suites.hidden_npcs.quests.contains(id)) { suites.hidden_npcs.quests.at(id).progress(source, amount, hard_set); }
}

void QuestTracker::reset(QuestType type, int id) {
	if (type == QuestType::npc) { suites.npc.reset(id); }
	if (type == QuestType::item) { suites.item.reset(id); }
	if (type == QuestType::standard) { suites.standard.reset(id); }
	if (type == QuestType::inspectable) { suites.inspectables.reset(id); }
	if (type == QuestType::destructibles) { suites.destructibles.reset(id); }
	if (type == QuestType::time_trials) { suites.time_trials.reset(id); }
	if (type == QuestType::fetch_text) { suites.fetch_text.reset(id); }
	if (type == QuestType::cutscene) { suites.cutscene.reset(id); }
	if (type == QuestType::hidden_npcs) { suites.hidden_npcs.reset(id); }
}

void QuestTracker::process(automa::ServiceProvider& svc, util::QuestKey key) {
	if (key.type == 69) { svc.state_controller.actions.set(automa::Actions::print_stats); }
	if (key.type == 70) { svc.menu_controller.open_vendor_dialog(key.id); }
	if (key.type == 97) { svc.state_controller.actions.set(automa::Actions::delete_file); }
	if (key.type == 299) { svc.state_controller.actions.set(automa::Actions::end_demo); }
	// std::cout << "Processed: " << out.type << ", " << out.id << ", " << out.source_id << ", " << out.amount << ", " << out.hard_set << "\n";
	if (key.hard_set > 0) {
		while (get_progression(static_cast<QuestType>(key.type), key.id) < key.hard_set) { progress(static_cast<QuestType>(key.type), key.id, key.source_id, key.amount, true); }
	} else {
		progress(static_cast<QuestType>(key.type), key.id, key.source_id, key.amount);
	}
	svc.data.push_quest(key);
}

} // namespace fornani::quest
