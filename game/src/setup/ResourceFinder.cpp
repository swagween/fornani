
#include "fornani/setup/ResourceFinder.hpp"

namespace data {

ResourceFinder::ResourceFinder(char** argv) {
	paths.resources = find_directory(argv[0], "resources");
	paths.editor = find_directory(argv[0], "resources/editor");
	paths.levels = find_directory(argv[0], "resources/level");
	paths.out = find_directory(argv[0], "resources/editor/export");
	paths.screenshots = fs::path{argv[0]};
}

fs::path ResourceFinder::region_and_room() const { return fs::path{paths.region} / fs::path{paths.room_name}; }

fs::path ResourceFinder::find_directory(fs::path const& exe, std::string const& target) {
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

} // namespace data
