#include "gamescreen.h"

#include <prism/actorhandler.h>
#include <prism/collisionhandler.h>
#include <prism/sound.h>

#include "levelhandler.h"
#include "playerhandler.h"
#include "collision.h"
#include "magnethandler.h"

static void loadGameScreen() {
	loadGameCollisions();
	instantiateActor(LevelHandler);
	instantiateActor(PlayerHandler);
	instantiateActor(MagnetHandler);
	
	if (getCurrentLevel() == 0) {
		playTrack(3);
	}
	// activateCollisionHandlerDebugMode();
}


Screen GameScreen = {
.mLoad = loadGameScreen,
};