
#include "editor/automa/PopupHandler.hpp"
#include <imgui.h>
#include <fornani/entity/EntitySet.hpp>
#include <fornani/service/ServiceProvider.hpp>
#include <fornani/setup/ResourceFinder.hpp>
#include <fornani/utils/Random.hpp>
#include <string>
#include "editor/gui/Console.hpp"
#include "editor/tool/Tool.hpp"

namespace pi {

void PopupHandler::launch(fornani::automa::ServiceProvider& svc, fornani::ResourceFinder& finder, Console& console, char const* label, std::unique_ptr<Tool>& tool, int room_id) {

	if (ImGui::BeginPopupModal("Inspectable Message", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
		m_is_open = true;
		static bool activate_on_contact{};
		static bool instant{};
		static char keybuffer[128] = "";
		static char msgbuffer[512] = "";

		ImGui::InputTextWithHint("Key", "Title (invisible in-game; must be unique per room)", keybuffer, IM_ARRAYSIZE(keybuffer));
		ImGui::Separator();

		ImGui::InputTextWithHint("Message", "Type message here...", msgbuffer, IM_ARRAYSIZE(msgbuffer));
		ImGui::Separator();

		ImGui::Checkbox("Activate on contact?", &activate_on_contact);
		ImGui::Checkbox("Instant text output when read?", &instant);
		ImGui::SameLine();

		if (ImGui::Button("Create")) {
			m_is_open = false;
			// switch to entity tool, and store the specified inspectable for placement
			tool = std::move(std::make_unique<EntityEditor>(EntityMode::placer));
			tool->ent_type = EntityType::inspectable;
			tool->current_entity = std::make_unique<fornani::Inspectable>(svc, std::vector<std::vector<fornani::gui::BasicMessage>>{{fornani::gui::BasicMessage{std::string{msgbuffer}}}},
																		  std::vector<std::vector<fornani::gui::BasicMessage>>{}, activate_on_contact, std::string{keybuffer}, 0, instant);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Close")) {
			m_is_open = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	if (ImGui::BeginPopupModal("Platform Specifications", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		m_is_open = true;

		static dj::Json in_specs{};
		in_specs = *dj::Json::from_file(std::string{finder.paths.resources.string() + "/data/level/platform.json"}.c_str());
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
		for (auto const& [key, count] : in_specs.as_object()) {
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
			m_is_open = false;
			// switch to entity tool, and store the specified portal for placement
			tool = std::move(std::make_unique<EntityEditor>(EntityMode::placer));
			tool->ent_type = EntityType::platform;
			tool->current_entity = std::make_unique<fornani::Platform>(svc, sf::Vector2<std::uint32_t>{static_cast<std::uint32_t>(x), static_cast<std::uint32_t>(y)}, extent, type, start);
			console.add_log(std::string{"Initialized platform with type " + type}.c_str());
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Close")) {
			m_is_open = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	if (ImGui::BeginPopupModal("Portal Specifications", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		m_is_open = true;

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
			m_is_open = false;
			// switch to entity tool, and store the specified portal for placement
			tool = std::move(std::make_unique<EntityEditor>(EntityMode::placer));
			tool->current_entity = std::make_unique<fornani::Portal>(svc, sf::Vector2u{static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height)}, activate_on_contact, already_open, room_id, destination, locked, key_id);
			console.add_log(std::string{"Room ID: " + std::to_string(room_id)}.c_str());
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Close")) {
			m_is_open = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	if (ImGui::BeginPopupModal("Enemy Specifications", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		m_is_open = true;
		static int selected{};
		static int variant{};

		ImGui::Text("Type:");
		for (auto const& [key, entry] : svc.data.enemy.as_object()) {
			if (ImGui::Selectable(std::string{key}.c_str(), selected == entry["metadata"]["id"].as<int>(), ImGuiSelectableFlags_DontClosePopups)) { selected = entry["metadata"]["id"].as<int>(); }
		}
		ImGui::InputInt("Variant", &variant);
		if (ImGui::Button("Create")) {
			m_is_open = false;
			tool = std::move(std::make_unique<EntityEditor>(EntityMode::placer));
			tool->current_entity = std::make_unique<fornani::Enemy>(svc, selected, variant);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Close")) {
			m_is_open = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	if (ImGui::BeginPopupModal("NPC Specifications", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		m_is_open = true;

		static dj::Json in_specs{};
		in_specs = *dj::Json::from_file(std::string{finder.paths.resources.string() + "/data/npc/npc_data.json"}.c_str());
		assert(!in_specs.is_null());

		static int id{room_id};
		static std::string label{};
		static std::vector<int> quantities{};
		static int iterations{};
		static bool face_player{};
		static std::vector<std::vector<int>> suites{};

		ImGui::Separator();
		ImGui::Text("Name");
		auto ctr{0};
		static int selected{};
		for (auto const& [key, count] : in_specs.as_object()) {
			if (ImGui::Selectable(std::string{key}.c_str(), selected == ctr, ImGuiSelectableFlags_DontClosePopups)) {
				label = std::string{key};
				selected = ctr;
			}
			++ctr;
		}
		ImGui::InputInt("Number of Possible Conversations", &iterations);
		suites.resize(static_cast<std::size_t>(iterations));
		quantities.resize(static_cast<std::size_t>(iterations));
		for (auto i = 0; i < iterations; ++i) {
			ImGui::PushID(i);
			ImGui::SeparatorText(std::to_string(i).c_str());
			ImGui::InputInt("Number of Suites", &quantities.at(i));
			suites.at(i).resize(static_cast<std::size_t>(quantities.at(i)));
			for (auto j = 0; j < quantities.at(i); ++j) {
				ImGui::SeparatorText(std::to_string(j).c_str());
				ImGui::PushID(j);
				ImGui::InputInt(std::to_string(j).c_str(), &suites.at(i).at(j));
				ImGui::PopID();
			}
			ImGui::PopID();
		}

		if (ImGui::Button("Create")) {
			m_is_open = false;
			tool = std::move(std::make_unique<EntityEditor>(EntityMode::placer));
			tool->current_entity = std::make_unique<fornani::NPC>(svc, id, label, suites);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Close")) {
			m_is_open = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	if (ImGui::BeginPopupModal("Destructible Specifications", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		m_is_open = true;
		static int id{};
		ImGui::InputInt("ID", &id);
		if (ImGui::Button("Create")) {
			m_is_open = false;
			tool = std::move(std::make_unique<EntityEditor>(EntityMode::placer));
			tool->current_entity = std::make_unique<fornani::Destructible>(svc, id);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Close")) {
			m_is_open = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	if (ImGui::BeginPopupModal("Animator Specifications", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		m_is_open = true;
		static int id{};
		static char const* label{"small"};
		static char const* types[2] = {"small", "large"};
		ImGui::InputInt("ID (fix later, should be image buttons)", &id);
		auto ctr{0};
		if (ImGui::BeginCombo("Type", types[0])) {
			for (auto const& t : types) {
				if (ImGui::Selectable(t)) { label = t; }
			}
			ImGui::EndCombo();
		}
		if (ImGui::Button("Create")) {
			m_is_open = false;
			tool = std::move(std::make_unique<EntityEditor>(EntityMode::placer));
			tool->current_entity = std::make_unique<fornani::Animator>(svc, id, label);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Close")) {
			m_is_open = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	if (ImGui::BeginPopupModal("Bed Specifications", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		m_is_open = true;
		static bool flipped{};
		ImGui::Checkbox("Flipped", &flipped);
		ImGui::SameLine();
		help_marker("By default, the foot of the bed is on the right");
		if (ImGui::Button("Create")) {
			m_is_open = false;
			tool = std::move(std::make_unique<EntityEditor>(EntityMode::placer));
			tool->current_entity = std::make_unique<fornani::Bed>(svc, 0, flipped);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Close")) {
			m_is_open = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	if (ImGui::BeginPopupModal("Vine Specifications", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		m_is_open = true;
		static int length{};
		static int link_index{-1};
		static bool foreground{};
		static bool reversed{};
		ImGui::InputInt("Length", &length);
		ImGui::InputInt("Platform Link Index", &link_index);
		link_index = std::clamp(link_index, -1, length - 1);
		ImGui::SameLine();
		help_marker("(Optional) Defines which link will contain a platform for the player to jump on. -1 means no platform. It is possible for vines to have multiple platforms, but this editor doesn't yet support this edge case. To do "
					"this, edit the level.json manually.");
		ImGui::Checkbox("Foreground?", &foreground);
		ImGui::Checkbox("Reversed?", &reversed);
		ImGui::SameLine();
		help_marker("(not yet supported)");
		if (ImGui::Button("Create")) {
			m_is_open = false;
			tool = std::move(std::make_unique<EntityEditor>(EntityMode::placer));
			tool->current_entity = std::make_unique<fornani::Vine>(svc, length, 2, foreground, reversed, std::vector<int>{link_index});
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Close")) {
			m_is_open = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	if (ImGui::BeginPopupModal("Chest Specifications", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		m_is_open = true;

		static int id{fornani::random::random_range(10000, 99999)};
		static int modifier{};
		static int type{};
		static char const* label{"item_label"};
		static char const* types[3] = {"gun", "orb", "item"};
		static bool custom{};

		ImGui::Text("ID: %i", id);
		ImGui::SameLine();
		ImGui::Checkbox("##custom", &custom);
		ImGui::SameLine();
		help_marker("Unique ID for the chest. No need to change this unless you want to.");
		if (custom) { ImGui::InputInt("ID", &id); }
		auto ctr{0};
		if (ImGui::BeginCombo("Type", types[type])) {
			for (auto const& t : types) {
				if (ImGui::Selectable(t)) { type = ctr; }
				++ctr;
			}
			ImGui::EndCombo();
		}

		ImGui::SameLine();
		help_marker("0 for gun, 1 for orbs, 2 for item");

		if (type == 0 || type == 2) {
			if (ImGui::BeginCombo("Contents", label, ImGuiComboFlags_HeightLargest)) {
				switch (type) {
				case 0:
					for (auto const& gun : svc.data.weapon.as_object()) {
						if (ImGui::Selectable(gun.first.c_str())) {
							modifier = gun.second["metadata"]["id"].as<int>();
							label = gun.first.c_str();
							ImGui::SetItemDefaultFocus();
						}
					}
					break;
				case 2:
					for (auto const& item : svc.data.item.as_object()) {
						if (ImGui::Selectable(item.first.c_str())) {
							modifier = item.second["id"].as<int>();
							label = item.first.c_str();
							ImGui::SetItemDefaultFocus();
						}
					}
					break;
				default: break;
				}
				ImGui::EndCombo();
			}
		} else if (type == 1) {
			ImGui::SliderInt("##rarity", &modifier, 1, 100);
			ImGui::SameLine();
			help_marker("Defines the rarity of the contents. 1 is most common, 100 is rarest.");
		}

		if (ImGui::Button("Create")) {
			m_is_open = false;
			tool = std::move(std::make_unique<EntityEditor>(EntityMode::placer));
			tool->current_entity = std::make_unique<fornani::Chest>(svc, type, modifier, id);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Close")) {
			m_is_open = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	if (ImGui::BeginPopupModal("Switch Button Specifications", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		m_is_open = true;
		static int id{};
		static int type{};
		ImGui::InputInt("ID", &id);
		static char const* types[4] = {"toggler", "permanent", "movable", "alternator"};

		auto ctr{0};
		if (ImGui::BeginCombo("Type", types[type])) {
			for (auto const& t : types) {
				if (ImGui::Selectable(t)) { type = ctr; }
				++ctr;
			}
			ImGui::EndCombo();
		}

		if (ImGui::Button("Create")) {
			m_is_open = false;
			tool = std::move(std::make_unique<EntityEditor>(EntityMode::placer));
			tool->current_entity = std::make_unique<fornani::SwitchButton>(svc, id, type);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Close")) {
			m_is_open = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	if (ImGui::BeginPopupModal("Switch Block Specifications", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		m_is_open = true;
		static int id{};
		static int type{};
		ImGui::InputInt("ID", &id);
		static char const* types[4] = {"toggler", "permanent", "movable", "alternator"};

		auto ctr{0};
		if (ImGui::BeginCombo("Type", types[type])) {
			for (auto const& t : types) {
				if (ImGui::Selectable(t)) { type = ctr; }
				++ctr;
			}
			ImGui::EndCombo();
		}

		if (ImGui::Button("Create")) {
			m_is_open = false;
			tool = std::move(std::make_unique<EntityEditor>(EntityMode::placer));
			tool->current_entity = std::make_unique<fornani::SwitchBlock>(svc, id, type);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Close")) {
			m_is_open = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	if (ImGui::BeginPopupModal("Water Specifications", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		m_is_open = true;
		static int id{};
		static bool toxic{};
		static int width{0};
		static int height{0};

		ImGui::InputInt("ID", &id);
		ImGui::Checkbox("Toxic", &toxic);

		ImGui::InputInt("Width", &width);
		ImGui::InputInt("Height", &height);

		if (ImGui::Button("Create")) {
			m_is_open = false;
			tool = std::move(std::make_unique<EntityEditor>(EntityMode::placer));
			tool->current_entity = std::make_unique<fornani::Water>(svc, sf::Vector2u{static_cast<unsigned int>(width), static_cast<unsigned int>(height)}, id, toxic);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Close")) {
			m_is_open = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	if (ImGui::BeginPopupModal("Timer Block Specifications", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		m_is_open = true;
		static int id{};
		static int type{};
		ImGui::InputInt("ID", &id);
		static char const* types[2] = {"start", "finish"};

		auto ctr{0};
		if (ImGui::BeginCombo("Type", types[type])) {
			for (auto const& t : types) {
				if (ImGui::Selectable(t)) { type = ctr; }
				++ctr;
			}
			ImGui::EndCombo();
		}

		if (ImGui::Button("Create")) {
			m_is_open = false;
			tool = std::move(std::make_unique<EntityEditor>(EntityMode::placer));
			tool->current_entity = std::make_unique<fornani::TimerBlock>(svc, id, type);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Close")) {
			m_is_open = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	if (ImGui::BeginPopupModal("Turret Specifications", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		m_is_open = true;
		static int type{};
		static int pattern{};
		static int hv{};
		static char const* types[2] = {"laser", "projectile"};
		static char const* patterns[3] = {"constant", "repeater", "triggerable"};
		static char const* dirs[4] = {"up", "down", "left", "right"};
		static float delay{1.f};
		static int duration{256};

		if (ImGui::BeginCombo("Type", types[type])) {
			for (auto [i, t] : std::views::enumerate(types)) {
				if (ImGui::Selectable(t)) { type = i; }
			}
			ImGui::EndCombo();
		}
		if (ImGui::BeginCombo("Pattern", patterns[pattern])) {
			for (auto [i, t] : std::views::enumerate(patterns)) {
				if (ImGui::Selectable(t)) { pattern = i; }
			}
			ImGui::EndCombo();
		}
		if (ImGui::BeginCombo("Direction", dirs[hv])) {
			for (auto [i, t] : std::views::enumerate(dirs)) {
				if (ImGui::Selectable(t)) { hv = i; }
			}
			ImGui::EndCombo();
		}
		ImGui::SliderFloat("Delay", &delay, 0.0, 1.0, "%.1f");
		ImGui::InputInt("Duration", &duration);

		if (ImGui::Button("Create")) {
			m_is_open = false;
			tool = std::move(std::make_unique<EntityEditor>(EntityMode::placer));
			tool->current_entity = std::make_unique<fornani::Turret>(svc, 0, static_cast<fornani::TurretType>(type), static_cast<fornani::TurretPattern>(pattern), fornani::CardinalDirection{hv}, fornani::TurretSettings{delay, duration});
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Close")) {
			m_is_open = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	if (ImGui::BeginPopupModal("Light Specifications", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		m_is_open = true;
		static int id{fornani::random::random_range(10000, 99999)};
		static char const* types[2] = {"lantern", "candlelight"};
		static int type{};

		auto ctr{0};
		if (ImGui::BeginCombo("Type", types[type])) {
			for (auto const& t : types) {
				if (ImGui::Selectable(t)) { type = ctr; }
				++ctr;
			}
			ImGui::EndCombo();
		}

		if (ImGui::Button("Create")) {
			m_is_open = false;
			tool = std::move(std::make_unique<EntityEditor>(EntityMode::placer));
			tool->current_entity = std::make_unique<fornani::Light>(svc, id, types[type]);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Close")) {
			m_is_open = false;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	if (ImGui::BeginPopupModal("Cutscene Trigger Specifications", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
		m_is_open = true;

		static int width{0};
		static int height{0};
		static int id{0};

		ImGui::InputInt("Width", &width);
		ImGui::Separator();
		ImGui::NewLine();

		ImGui::InputInt("Height", &height);
		ImGui::Separator();
		ImGui::NewLine();

		ImGui::InputInt("Cutscene ID", &id);
		ImGui::Separator();
		ImGui::NewLine();

		if (ImGui::Button("Create")) {
			m_is_open = false;
			// switch to entity tool, and store the specified portal for placement
			tool = std::move(std::make_unique<EntityEditor>(EntityMode::placer));
			tool->current_entity = std::make_unique<fornani::CutsceneTrigger>(svc, sf::Vector2u{static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height)}, id);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Close")) {
			m_is_open = false;
			ImGui::CloseCurrentPopup();
		}
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
