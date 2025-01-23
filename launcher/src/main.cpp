
#include "launcher/LauncherApplication.hpp"

#include <steam/steam_api.h>
#include <iostream>

#ifndef FORNANI_STEAM_APP_ID
#error "FORNANI_STEAM_APP_ID was not defined!"
#endif

#if defined(FORNANI_STEAM_APP_ID) && FORNANI_STEAM_APP_ID < 0
#error "FORNANI_STEAM_APP_ID was defined as a negative number!"
#endif

static constexpr const char * logFile{"fornani.log"};

int main(int argc, char** argv) {
	assert(argc > 0);

	// TODO: Maybe move this into a config file?
	auto config = fornani::logger::Config{};
	// Required to initialize the logger for the application. This must also stay outside any try/catch block.
	auto logger = fornani::logger::Instance{logFile, config};


	game::LauncherApplication app{argv};
	app.init(argv);

	std::cout << "Current passed steam ID: " << FORNANI_STEAM_APP_ID << "\n";

	if (SteamAPI_RestartAppIfNecessary(FORNANI_STEAM_APP_ID)) {
		std::cout << "Re-launching through Steam.\n";
		return EXIT_SUCCESS;
	}
	SteamErrMsg errMsg;
	if (SteamAPI_InitEx(&errMsg) != k_ESteamAPIInitResult_OK) {
		std::cout << "Failed to init Steam: " << static_cast<const char *>(errMsg) << "\n";
		return EXIT_FAILURE;
	}

	std::cout << "SteamAPI has been initialized.\n";
	app.launch(argv);

	return EXIT_SUCCESS;
}
