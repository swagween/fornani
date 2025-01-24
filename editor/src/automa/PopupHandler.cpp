
#include "editor/automa/PopupHandler.hpp"
#include "editor/tool/Tool.hpp"
#include "editor/gui/Console.hpp"
#include "fornani/setup/ResourceFinder.hpp"
#include <imgui.h>
#include <string>

namespace pi {

void PopupHandler::launch(data::ResourceFinder& finder, Console& console, char const* label, std::unique_ptr<Tool>& tool) {
	if (ImGui::BeginPopupModal("Inspectable Message", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {

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
			tool->current_entity = std::make_unique<Inspectable>(activate_on_contact, std::string{keybuffer}, std::vector<std::vector<std::string>>{{msgbuffer}}, std::vector<std::vector<std::string>>{}, 0);
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
			tool->current_entity = std::make_unique<Platform>(sf::Vector2<uint32_t>{static_cast<uint32_t>(x), static_cast<uint32_t>(y)}, extent, type, start);
			console.add_log(std::string{"Initialized platform with type " + type}.c_str());
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
