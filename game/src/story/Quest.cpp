
#include <fornani/story/Quest.hpp>

namespace fornani {

QuestProgression::QuestProgression(std::vector<std::pair<QuestIdentifier, ProgressionState>> status) {
	for (auto& s : status) { progressions.insert({s.first, s.second}); }
}

void QuestProgression::progress(QuestIdentifier const identifier, int const amount, int const source) {
	if (!progressions.contains(identifier)) { progressions.insert({identifier, 0}); }
	if (std::find(m_sources.begin(), m_sources.end(), source) == m_sources.end() || source == -1) {
		progressions.at(identifier) += amount;
		if (source != -1) { m_sources.push_back(source); }
	}
}
void QuestProgression::set_progression(QuestIdentifier const identifier, int const amount, std::vector<int> const sources) {
	if (!progressions.contains(identifier)) { return; }
	progressions.at(identifier) = amount;
	m_sources = sources;
}

Quest::Quest(dj::Json const& in) {
	m_title = in["title"].as_string();
	m_tag = in["tag"].as_string();
	m_progression_target = in["target"].as<int>();
	for (auto const& objective : in["objectives"].as_array()) { m_objectives.push_back(objective.as_string()); }
	if (in["subquests"].is_array()) {
		m_subquests = std::vector<int>{};
	} else {
		m_subquests = {0};
	}
	for (auto const& subquest : in["subquests"].as_array()) { m_subquests->push_back(subquest.as<int>()); }
}

auto Quest::get_objectives() const -> std::string {
	auto ret = std::string{};
	for (auto const& objective : m_objectives) { ret += objective + "\n"; }
	return ret;
}

QuestRegistry::QuestRegistry(ResourceFinder& finder) {
	auto path = std::filesystem::path{finder.resource_path().data()} / "data" / "quest" / "quests.json";
	auto quest_data_result = dj::Json::from_file(path.string());
	if (!quest_data_result) {
		NANI_LOG_ERROR(m_logger, "Failed to load quest data from path {}.", path.string());
		return;
	}
	auto quest_data = std::move(*quest_data_result);
	auto index = 0;
	for (auto const& entry : quest_data.as_array()) {
		m_registry.insert({index, Quest{entry}});
		m_indeces.insert({entry["tag"].as_string(), index});
		++index;
	}
}

QuestTable::QuestTable(QuestRegistry& registry) : m_registry(&registry) {}

void QuestTable::serialize(dj::Json& to_save) {
	to_save["quests"] = dj::Json::empty_array();
	for (auto const& [tag, quest] : m_quests) {
		dj::Json out_quest{};
		out_quest["tag"] = tag;
		for (auto& progression : quest.progressions) {
			dj::Json out_status{};
			out_status.push_back(progression.first);
			out_status.push_back(progression.second);
			out_quest["status"].push_back(out_status);
		}
		to_save["quests"].push_back(out_quest);
	}
}

void QuestTable::unserialize(dj::Json const& from_save) {
	m_quests.clear();
	for (auto const& in_quest : from_save["quests"].as_array()) {
		auto status = std::vector<std::pair<int, ProgressionState>>{};
		for (auto const& s : in_quest["status"].as_array()) { status.push_back({s[0].as<int>(), s[1].as<int>()}); }
		m_quests.insert({in_quest["tag"].as_string(), QuestProgression{status}});
	}
}

void QuestTable::start_quest(std::string_view tag, std::vector<std::pair<QuestIdentifier, ProgressionState>> const identifiers) { m_quests.insert({tag.data(), QuestProgression{identifiers}}); }

void QuestTable::progress_quest(std::string_view tag, int const amount, int const source, QuestIdentifier const identifier) {
	if (!m_quests.contains(tag.data())) { start_quest(tag, {{identifier, 0}}); }
	m_quests.at(tag.data()).progress(identifier, amount, source);
}

void QuestTable::set_quest_progression(std::string_view tag, QuestIdentifier const identifier, int const amount, std::vector<int> sources) {
	if (m_quests.contains(tag.data())) { m_quests.at(tag.data()).set_progression(identifier, amount, sources); }
}

auto QuestTable::print_progressions(std::string_view tag, std::string_view identifier) const -> std::string {
	if (!m_quests.contains(tag.data())) { return "<null>"; }
	auto ret = std::string{};
	for (auto const& p : m_quests.at(tag.data()).progressions) {
		std::string next = tag.data();
		auto id = identifier.empty() ? std::to_string(p.first) : identifier.data();
		ret += next + ": " + id + " - " + std::to_string(p.second) + "\n";
	}
	return ret;
}

} // namespace fornani
