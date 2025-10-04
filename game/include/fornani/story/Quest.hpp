
#pragma once

#include <djson/json.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/setup/ResourceFinder.hpp>
#include <functional>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace fornani {
struct Subquest {
	std::string tag{};
	int id{};

	bool operator==(Subquest const& other) const { return tag == other.tag && id == other.id; }
};
} // namespace fornani

namespace std {
template <>
struct hash<fornani::Subquest> {
	std::size_t operator()(fornani::Subquest const& s) const noexcept {
		std::size_t h1 = std::hash<std::string>{}(s.tag);
		std::size_t h2 = std::hash<int>{}(s.id);

		return h1 ^ (h2 << 1);
	}
};
} // namespace std

namespace fornani {

using ProgressionState = int;
using QuestIdentifier = int;

struct QuestProgression {
	QuestProgression(std::vector<std::pair<QuestIdentifier, ProgressionState>> status);
	void progress(QuestIdentifier const identifier, int const amount, int const source);
	void progress(Subquest const identifier, int const amount, int const source);
	void set_progression(QuestIdentifier const identifier, int const amount, std::vector<int> const sources);
	void set_progression(Subquest const identifier, int const amount, std::vector<int> const sources);

	[[nodiscard]] auto get_progression(QuestIdentifier const identifier = 0) const -> ProgressionState { return progressions.contains(identifier) ? progressions.at(identifier) : 0; }
	[[nodiscard]] auto get_progression(Subquest const identifier) const -> ProgressionState { return subquest_progressions.contains(identifier) ? subquest_progressions.at(identifier) : 0; }
	[[nodiscard]] auto get_size() const -> std::size_t { return progressions.size(); }

	std::unordered_map<int, ProgressionState> progressions{};
	std::unordered_map<Subquest, ProgressionState> subquest_progressions{};

  private:
	std::vector<int> m_sources{};
};

/* Used by in-game entities, such as NPCs and CutsceneTriggers, that depend on a certain quest being progressed a certain amount. */
struct QuestContingency {
	std::string tag{};
	int requirement{};
	bool strict{};
	QuestContingency(std::string_view to_tag, int const to_req, bool to_strict) : tag{to_tag}, requirement{to_req}, strict{to_strict} {}
	QuestContingency(dj::Json const& in);
	void serialize(dj::Json& out) const;

	// for the dialogue editor
	bool delete_me{};
};

class Quest {
  public:
	Quest() {}
	Quest(dj::Json const& in);

	[[nodiscard]] auto get_title() const -> std::string_view { return m_title; }
	[[nodiscard]] auto get_tag() const -> std::string_view { return m_tag; }
	[[nodiscard]] auto get_objectives() const -> std::string;

  private:
	std::string m_title{"Null"};
	std::string m_tag{"null"};
	std::vector<std::string> m_objectives{};
	std::optional<std::vector<Subquest>> m_subquests{};
	int m_progression_target{};
};

/* Permanent, determined by the design of the game. */
class QuestRegistry {
  public:
	QuestRegistry(ResourceFinder& finder);

	[[nodiscard]] auto get_quest_metadata(int index) const& -> Quest { return m_registry.contains(index) ? m_registry.at(index) : null_quest; }
	[[nodiscard]] auto get_index_from_tag(std::string_view tag) const -> std::size_t { return m_indeces.contains(tag.data()) ? m_indeces.at(tag.data()) : -1; }
	[[nodiscard]] auto get_size() const -> std::size_t { return m_registry.size(); }

  private:
	std::unordered_map<int, Quest> m_registry{};
	std::unordered_map<std::string, std::size_t> m_indeces{};
	Quest null_quest{};

	io::Logger m_logger{"quest"};
};

/* Temporal, determined by the contents of the player's save file, and modified in-game. */
class QuestTable {
  public:
	QuestTable(QuestRegistry& registry);
	void serialize(dj::Json& to_save);
	void unserialize(dj::Json const& from_save);

	void progress_quest(std::string_view tag, int const amount, int const source, QuestIdentifier const identifier = 0);
	void progress_quest(std::string_view tag, Subquest const subquest, int const amount, int const source, QuestIdentifier const identifier = 0);
	void set_quest_progression(std::string_view tag, QuestIdentifier const identifier, int const amount, std::vector<int> sources);
	void set_quest_progression(std::string_view tag, Subquest const subquest, int const amount, std::vector<int> const sources, QuestIdentifier const identifier = 0);

	[[nodiscard]] auto get_quest_progression(std::string_view tag, QuestIdentifier const identifier = 0) const -> ProgressionState { return m_quests.contains(tag.data()) ? m_quests.at(tag.data()).get_progression(identifier) : 0; }
	[[nodiscard]] auto get_quest_progression(std::string_view tag, Subquest const identifier) const -> ProgressionState { return m_quests.contains(tag.data()) ? m_quests.at(tag.data()).get_progression(identifier) : 0; }
	[[nodiscard]] auto print_progressions(std::string_view tag, std::string_view identifier = "") const -> std::string;
	[[nodiscard]] auto are_contingencies_met(std::vector<QuestContingency> const& set) const -> bool;

  private:
	void start_quest(std::string_view tag, std::vector<std::pair<QuestIdentifier, ProgressionState>> status);
	QuestRegistry* m_registry;
	std::unordered_map<std::string, QuestProgression> m_quests{};
};

} // namespace fornani
