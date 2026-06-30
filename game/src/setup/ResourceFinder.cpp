
#include <platform_folders.h>
#include <fornani/io/Codec.hpp>
#include <fornani/io/FileSerializer.hpp>
#include <fornani/setup/ResourceFinder.hpp>

namespace fornani {

ResourceFinder::ResourceFinder(char** argv) {

	// generate save files on target machine
	auto root = fs::path{sago::getDataHome()};
	paths.save = root / "fornani" / "save";
	paths.config = root / "fornani" / "config";
	paths.screenshots = root / "fornani" / "screenshots";
	std::filesystem::create_directories(paths.save);
	std::filesystem::create_directories(paths.config);
	std::filesystem::create_directories(paths.screenshots);

	paths.resources = find_directory(argv[0], fs::path{"resources"});
	paths.editor = find_directory(argv[0], paths.resources / fs::path{"editor"});
	paths.levels = find_directory(argv[0], paths.resources / fs::path{"level"});

	NANI_LOG_INFO(m_logger, "Resource Directory............: {}", paths.resources.string());
	NANI_LOG_INFO(m_logger, "Local User Save Directory.....: {}", paths.save.string());
	NANI_LOG_INFO(m_logger, "Local User Config Directory...: {}", paths.config.string());
}

fs::path ResourceFinder::region_and_room() const { return fs::path{paths.region} / fs::path{paths.room_name}; }

fs::path ResourceFinder::find_directory(fs::path const& exe, fs::path const& target) {
	auto execpy = fs::path{exe};
	auto check = [target](fs::path const& prefix) {
		auto path = prefix / target;
		if (fs::is_directory(path)) { return path; }
		return fs::path{};
	};
	while (!execpy.empty()) {
		if (auto ret = check(execpy); !ret.empty()) { return ret; }
		auto parent = execpy.parent_path();
		if (execpy == parent) { break; }
		execpy = std::move(parent);
	}
	return {};
}

void ResourceFinder::ensure_file_exists(fs::path const& target, fs::path const& template_file) const {
	if (fs::exists(target)) {
		NANI_LOG_INFO(m_logger, "Local user data found: {}", target.string());
		return;
	}
	fs::create_directories(target.parent_path());
	try {
		fs::copy_file(template_file, target);
		NANI_LOG_INFO(m_logger, "Created Save File: {}", target.string());
	} catch (std::exception const& e) { NANI_LOG_ERROR(m_logger, "Failed to create Save File: {}. exception: {}", target.string(), e.what()); }
}

void ResourceFinder::ensure_save_exists(fs::path const& sav_path, fs::path const& template_path) const {
	if (fs::exists(sav_path)) {
		NANI_LOG_INFO(m_logger, ".sav file exists: {}", sav_path.string());
		return;
	}

	try {
		auto in = dj::Json::from_file(template_path.string());
		if (!in) { return; }

		std::string json = in->serialize();

		std::ofstream out(sav_path, std::ios::binary);
		if (!out) {
			NANI_LOG_ERROR(m_logger, "Failed to create Save File: {}", sav_path.string());
			return;
		}
		if (!codec::encode(json, out)) {
			NANI_LOG_ERROR(m_logger, "Failed to write Save File: {}", sav_path.string());
			return;
		}

	} catch (std::exception const& e) { NANI_LOG_ERROR(m_logger, "Failed to create Save File: {}. exception: {}", sav_path.string(), e.what()); }
}

void ResourceFinder::overwrite_save(fs::path const& sav_path, fs::path const& template_path) const {
	try {
		auto in = dj::Json::from_file(template_path.string());
		if (!in) {
			NANI_LOG_ERROR(m_logger, "Failed to load template: {}", template_path.string());
			return;
		}

		std::string json = in->serialize();

		// Write to temp file first
		fs::path tmp = sav_path;
		tmp += ".tmp";

		{
			std::ofstream out(tmp, std::ios::binary);
			if (!out) {
				NANI_LOG_ERROR(m_logger, "Failed to create temp save file: {}", tmp.string());
				return;
			}

			if (!codec::encode(json, out)) {
				NANI_LOG_ERROR(m_logger, "Failed to write temp save file: {}", tmp.string());
				return;
			}
		}

		// Atomically replace
		fs::rename(tmp, sav_path);

		NANI_LOG_INFO(m_logger, "Save overwritten: {}", sav_path.string());
	} catch (std::exception const& e) { NANI_LOG_ERROR(m_logger, "Failed to overwrite Save File: {}. exception: {}", sav_path.string(), e.what()); }
}

} // namespace fornani
