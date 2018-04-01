#include <prism/framerateselectscreen.h>
#include <prism/pvr.h>
#include <prism/physics.h>
#include <prism/file.h>
#include <prism/drawing.h>
#include <prism/log.h>
#include <prism/wrapper.h>
#include <prism/system.h>
#include <prism/stagehandler.h>
#include <prism/logoscreen.h>
#include <prism/mugentexthandler.h>

#include "titlescreen.h"
#include "gamescreen.h"
#include "levelhandler.h"
#include "introscreen.h"
#include "outroscreen.h"

#ifdef DREAMCAST
KOS_INIT_FLAGS(INIT_DEFAULT);

extern uint8 romdisk[];
KOS_INIT_ROMDISK(romdisk);

#endif


void exitGame() {
	shutdownPrismWrapper();

#ifdef DEVELOP
	abortSystem();
#else
	returnToMenu();
#endif
}

void setMainFileSystem() {
#ifdef DEVELOP
	setFileSystem("/pc");
#else
	setFileSystem("/cd");
#endif
}

int main(int argc, char** argv) {
	(void)argc;
	(void)argv;

	setGameName("Ballad of the Sauftyralian");
	setScreenSize(640, 480);
	
	initPrismWrapperWithConfigFile("assets/config.cfg");
	setMainFileSystem();	
	setFont("$/rd/fonts/segoe.hdr", "$/rd/fonts/segoe.pkg");

	logg("Check framerate");
	FramerateSelectReturnType framerateReturnType = selectFramerate();
	if (framerateReturnType == FRAMERATE_SCREEN_RETURN_ABORT) {
		exitGame();
	}
	
	resetLevelHandler();
	setLevel(9);
	setWrapperIsPausingTracksBetweenScreens(0);
	setWrapperTitleScreen(&TitleScreen);
	setScreenAfterWrapperLogoScreen(getLogoScreenFromWrapper());
	startScreenHandling(&TitleScreen);
	
	exitGame();
	
	return 0;
}


