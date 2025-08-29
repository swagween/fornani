
#pragma once

#include <djson/json.hpp>
#include <fornani/io/Logger.hpp>
#include <fornani/setup/ResourceFinder.hpp>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace fornani {

using ProgressionState = int;
using QuestIdentifier = int;

struct QuestProgression {
	QuestProgression(std::vector<std::pair<QuestIdentifier, ProgressionState>> status);
	void progress(QuestIdentifier const identifier, int const amount, int const source);
	void set_progression(QuestIdentifier const identifier, int const amount, std::vector<int> const sources);

	[[nodiscard]] auto get_progression(QuestIdentifier const identifier = 0) const -> ProgressionState { return progressions.contains(identifier) ? progressions.at(identifier) : 0; }
	[[nodiscard]] auto get_size() const -> std::size_t { return progressions.size(); }

	std::unordered_map<int, ProgressionState> progressions{};

  private:
	std::vector<int> m_sources{};
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
	std::optional<std::vector<int>> m_subquests{};
	int m_progression_target{};
};

/* Permanent, determined by the design of the game. */
class QuestRegistry {
  public:
	QuestRegistry(ResourceFinder& finder);

	[[nodiscard]] auto get_quest_metadata(int index) const& -> Quest { return m_registry.contains(index) ? m_registry.at(index) : null_quest; }
	[[nodiscard]] auto get_size() const -> std::size_t { return m_registry.size(); }

  private:
	std::unordered_map<int, Quest> m_registry{};
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
	void set_quest_progression(std::string_view tag, QuestIdentifier const identifier, int const amount, std::vector<int> sources);

	[[nodiscard]] auto get_quest_progression(std::string_view tag, QuestIdentifier const identifier = 0) const -> ProgressionState { return m_quests.contains(tag.data()) ? m_quests.at(tag.data()).get_progression(identifier) : 0; }

  private:
	void start_quest(std::string_view tag, std::vector<std::pair<QuestIdentifier, ProgressionState>> status);
	QuestRegistry* m_registry;
	std::unordered_map<std::string, QuestProgression> m_quests{};
};

} // namespace fornani
