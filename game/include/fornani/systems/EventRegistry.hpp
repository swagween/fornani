
#pragma once

#include <fornani/events/ConsoleEvent.hpp>
#include <fornani/events/EventBase.hpp>
#include <fornani/events/GameplayEvent.hpp>
#include <fornani/events/InventoryEvent.hpp>
#include <fornani/events/SystemEvent.hpp>
#include <fornani/gui/console/Console.hpp>
#include <fornani/io/Logger.hpp>
#include <ksignal/ksignal.hpp>
#include <concepts>
#include <map>
#include <memory>
#include <typeindex>
#include <unordered_map>

namespace fornani {

struct EventRegistry {

	NPCVoiceCueEvent npc_voice_cue_event{};
	NPCPopConversationEvent npc_pop_conversation_event{};
	NPCPiggybackEvent npc_piggyback_event{};

	LaunchCutsceneEvent launch_cutscene_event{};
	OpenVendorEvent open_vendor_event{};
	AddMapMarkerEvent add_map_marker_event{};
	PlaySongEvent play_song_event{};
	StartBattleEvent start_battle_event{};

	AcquireItemFromConsoleEvent acquire_item_from_console_event{};
	AcquireItemEvent acquire_item_event{};
	ReadItemByIDEvent read_item_by_id_event{};
	EquipItemByIDEvent equip_item_by_id_event{};
	RevealItemByIDEvent reveal_item_by_id_event{};
	AcquireWeaponEvent acquire_weapon_event{};
	AcquireWeaponFromConsoleEvent acquire_weapon_from_console_event{};
	RemoveItemEvent remove_item_event{};
	RemoveWeaponByIDEvent remove_weapon_by_id_event{};

	LoadFileEvent load_file_event{};
	NewFileEvent new_file_event{};
	ReloadSaveEvent reload_save_event{};
	ReturnToMainMenuEvent return_to_main_menu_event{};
};

} // namespace fornani
