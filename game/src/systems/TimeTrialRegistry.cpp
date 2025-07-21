
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/systems/TimeTrialRegistry.hpp>

namespace fornani {

constexpr auto max_records_v = 8;

bool TimeTrialRegistry::register_time(automa::ServiceProvider& svc, int course, std::string_view tag, float time) {
	bool was_registered{true};
	if (m_trial_attempts.contains(course)) {
		auto win_counter = 0;
		auto& list = m_trial_attempts.at(course);
		if (list.size() < max_records_v) { // we have room in the registry
			list.push_back(TrialAttempt{tag.data(), time});
		} else {
			for (auto& record : list) {
				if (time < record.time) { ++win_counter; }
			}
			if (win_counter == 0) { // it's not fast enough
				was_registered = false;
			} else { // it's a new record, remove the worst time
				list.push_back(TrialAttempt{tag.data(), time});
				auto worst = list.back();
				for (auto& entry : list) {
					if (entry.time > worst.time) { worst = entry; }
				}
				std::erase_if(list, [worst](auto const& e) { return e.time == worst.time; });
			}
		}
	} else {
		m_trial_attempts.insert({course, std::vector<TrialAttempt>{TrialAttempt{tag.data(), time}}});
	}
	if (was_registered) {
		for (auto [index, track] : std::views::enumerate(svc.data.time_trial_data["trials"].as_array())) {
			if (track["course_id"].as<int>() == course) {
				svc.data.time_trial_data["trials"][index]["times"].set_value(dj::Json::empty_array());
				for (auto const& registered_time : m_trial_attempts.at(course)) {
					auto t = dj::Json{};
					dj::to_json(t["player_tag"], registered_time.player_tag);
					dj::to_json(t["time"], registered_time.time);
					svc.data.time_trial_data["trials"][index]["times"].push_back(t);
				}
			}
		}
		if (!svc.data.time_trial_data.to_file((svc.finder.resource_path() + "/data/save/time_trials.json").c_str())) {
			NANI_LOG_ERROR(m_logger, "Failed to save time trial record!");
		} else {
			NANI_LOG_INFO(m_logger, "New Record! Registered [{}]'s Time for course {}: {}", tag.data(), course, time);
		}
	} else {
		NANI_LOG_INFO(m_logger, "Failed to Register [{}]'s Time for course {}: {}. It wasn't fast enough!", tag.data(), course, time);
	}
	return was_registered;
}

std::optional<std::vector<TrialAttempt>> TimeTrialRegistry::readout_attempts(int const course) {
	if (m_trial_attempts.contains(course)) { return m_trial_attempts.at(course); }
	return std::nullopt;
}

} // namespace fornani
