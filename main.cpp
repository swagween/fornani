#include "src/setup/Application.hpp"
#include <steam/steam_api.h>
#include <iostream>
#define CCM_CONFIG_NO_FORCED_INLINE

int main(int argc, char** argv) {
	assert(argc > 0);
	fornani::Application app{argv};

	if (SteamAPI_RestartAppIfNecessary(FORNANI_STEAM_APP_ID)) {
		std::cout << "Re-launching through Steam." << std::endl;
		return 0;
	}
	SteamErrMsg errMsg;
	if (SteamAPI_InitEx(&errMsg) != k_ESteamAPIInitResult_OK) {
		std::cout << "Failed to init Steam: " << errMsg << std::endl;
		return 0;
	}
	app.launch(argv);
	return 0;
}
