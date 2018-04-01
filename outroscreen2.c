#include "outroscreen2.h"

#include <prism/blitzentity.h>
#include <prism/blitzcamerahandler.h>
#include <prism/blitzmugenanimation.h>
#include <prism/blitztimelineanimation.h>
#include <prism/blitzphysics.h>
#include <prism/screeneffect.h>
#include <prism/input.h>
#include <prism/timer.h>
#include <prism/sound.h>

#include "titlescreen.h"

static struct {

	MugenSpriteFile mSprites;
	BlitzTimelineAnimations mTimelineAnimations;

	int mIsFadingOut;
} gData;

static void loadOutroScreen() {
	gData.mSprites = loadMugenSpriteFileWithoutPalette("assets/outro/OUTRO2.sff");
	gData.mTimelineAnimations = loadBlitzTimelineAnimations("assets/outro/OUTRO2.taf");

	int entity = addBlitzEntity(makePosition(0, 0, 1));
	addBlitzMugenAnimationComponentStatic(entity, &gData.mSprites, 1, 0);
	addBlitzTimelineComponent(entity, &gData.mTimelineAnimations);
	playBlitzTimelineAnimation(entity, 1);

	addBlitzTimelineComponent(getBlitzCameraHandlerEntityID(), &gData.mTimelineAnimations);

	gData.mIsFadingOut = 0;
}

static void gotoTitleScreen(void* tData) {
	setTitleFadeIn();
	setNewScreen(&TitleScreen);
}

static void updateOutroScreen() {
	if (gData.mIsFadingOut) return;

	if (hasPressedStartFlank()) {
		stopTrack();
		playBlitzTimelineAnimation(getBlitzCameraHandlerEntityID(), 2);
		addTimerCB(300, gotoTitleScreen, NULL);
		gData.mIsFadingOut = 1;
	}
}

Screen OutroScreen2 = {
	.mLoad = loadOutroScreen,
	.mUpdate = updateOutroScreen,
};