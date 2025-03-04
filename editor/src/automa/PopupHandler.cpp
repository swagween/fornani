
#include "editor/automa/PopupHandler.hpp"
#include "editor/tool/Tool.hpp"
#include "editor/gui/Console.hpp"
#include "editor/canvas/entity/Inspectable.hpp"
#include "editor/canvas/entity/Platform.hpp"
#include "editor/canvas/entity/Portal.hpp"
#include "editor/canvas/entity/Enemy.hpp"
#include "fornani/setup/ResourceFinder.hpp"
#include <imgui.h>
#include <string>

namespace pi {

void PopupHandler::launch(fornani::data::ResourceFinder& finder, Console& console, char const* label, std::unique_ptr<Tool>& tool, int room_id) {
	if (ImGui::BeginPopupModal("Inspectable Message", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {

		static bool activate_on_contact{false};
		static char keybuffer[128] = "";
		static char msgbuffer[512] = "";

		ImGui::InputTextWithHint("Key", "Title (invisible in-game; must be unique per room)", keybuffer, IM_ARRAYSIZE(keybuffer));
		ImGui::Separator();

		ImGui::InputTextWithHint("Message", "Type message here...", msgbuffer, IM_ARRAYSIZE(msgbuffer));
		ImGui::Separator();

		ImGui::Checkbox("Activate on contact?", &activate_on_contact);
		ImGui::SameLine();

		if (ImGui::Button("Create")) {
			// switch to entity tool, and store the specified inspectable for placement
			tool = std::move(std::make_unique<EntityEditor>(EntityMode::placer));
			tool->ent_type = EntityType::inspectable;
			tool->current_entity = std::make_unique<Inspectable>(activate_on_contact, std::string{keybuffer}, std::vector<std::vector<std::string>>{{std::string{msgbuffer}}}, std::vector<std::vector<std::string>>{}, 0);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Close")) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}
	if (ImGui::BeginPopupModal("Platform Specifications", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {

		static dj::Json in_specs{};
		in_specs = dj::Json::from_file(std::string{finder.paths.resources.string() + "/data/level/platform.json"}.c_str());
		assert(!in_specs.is_null());

		static int x{1};
		static int y{1};
		static int extent{8};
		static int selected{};
		static std::string type{"standard_up_down"};
		static float start{};

		ImGui::InputInt("X Dimensions", &x);
		ImGui::InputInt("Y Dimensions", &y);
		ImGui::SameLine();
		ImGui::Separator();
		ImGui::NewLine();

		ImGui::InputInt("Extent", &extent);
		ImGui::Separator();
		ImGui::NewLine();

		// platforms.json stuff
		ImGui::Separator();
		ImGui::Text("Type:");
		auto ctr{0};
		for (auto const [key, count] : in_specs.object_view()) {
			if (ImGui::Selectable(std::string{key}.c_str(), selected == ctr, ImGuiSelectableFlags_DontClosePopups)) {
				type = std::string{key};
				selected = ctr;
			}
			++ctr;
		}
		ImGui::Separator();
		ImGui::NewLine();

		ImGui::InputFloat("Start", &start);
		ImGui::SameLine();
		help_marker("Must be a value between 0 and 1.");
		ImGui::Separator();
		ImGui::NewLine();

		if (ImGui::Button("Create")) {
			// switch to entity tool, and store the specified portal for placement
			tool = std::move(std::make_unique<EntityEditor>(EntityMode::placer));
			tool->ent_type = EntityType::platform;
			tool->current_entity = std::make_unique<Platform>(sf::Vector2<std::uint32_t>{static_cast<std::uint32_t>(x), static_cast<std::uint32_t>(y)}, extent, type, start);
			console.add_log(std::string{"Initialized platform with type " + type}.c_str());
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Close")) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}
	if (ImGui::BeginPopupModal("Portal Specifications", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {

		static int width{0};
		static int height{0};
		static int destination{0};
		static bool activate_on_contact{};
		static bool already_open{};
		static bool locked{};
		static int key_id{};

		ImGui::InputInt("Width", &width);
		ImGui::Separator();
		ImGui::NewLine();

		ImGui::InputInt("Height", &height);
		ImGui::Separator();
		ImGui::NewLine();

		ImGui::InputInt("Destination Room ID", &destination);
		ImGui::SameLine();
		help_marker("Must be an existing room before being activated in-game. By convention, choose a three-digit number where the first digit indicates the region.");
		ImGui::Separator();
		ImGui::NewLine();

		ImGui::Checkbox("Already open?", &already_open);
		ImGui::SameLine();
		help_marker("Only applies to 1x1 portals that are not activated on contact (doors). If true, the door will appear open.");
		ImGui::Separator();
		ImGui::NewLine();

		ImGui::Checkbox("Activate on contact?", &activate_on_contact);
		ImGui::SameLine();
		help_marker("If left unchecked, the player will have to inspect the portal to activate it.");
		ImGui::Separator();
		ImGui::NewLine();

		ImGui::Checkbox("Locked?", &locked);
		ImGui::SameLine();
		ImGui::InputInt("Key ID", &key_id);
		ImGui::Separator();
		ImGui::NewLine();

		if (ImGui::Button("Create")) {
			// switch to entity tool, and store the specified portal for placement
			tool = std::move(std::make_unique<EntityEditor>(EntityMode::placer));
			tool->current_entity = std::make_unique<Portal>(sf::Vector2u{static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height)}, activate_on_contact, already_open, room_id, destination, locked, key_id);
			console.add_log(std::string{"Room ID: " + std::to_string(room_id)}.c_str());
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Close")) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	}
	if (ImGui::BeginPopupModal("Enemy Specifications", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		static int id{};
		ImGui::InputInt("ID", &id);
		if (ImGui::Button("Create")) {
			tool = std::move(std::make_unique<EntityEditor>(EntityMode::placer));
			tool->current_entity = std::make_unique<Enemy>(id);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Close")) { ImGui::CloseCurrentPopup(); }
		ImGui::EndPopup();
	
	}
}

void PopupHandler::help_marker(char const* desc) {
	ImGui::TextDisabled("(?)");
	if (ImGui::IsItemHovered()) {
		ImGui::BeginTooltip();
		ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
		ImGui::TextUnformatted(desc);
		ImGui::PopTextWrapPos();
		ImGui::EndTooltip();
	}
}

} // namespace pi
