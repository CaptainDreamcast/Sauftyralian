#include "introscreen.h"

#include <prism/blitzentity.h>
#include <prism/blitzmugenanimation.h>
#include <prism/blitzphysics.h>
#include <prism/blitztimelineanimation.h>
#include <prism/input.h>
#include <prism/blitzcamerahandler.h>
#include <prism/timer.h>
#include <prism/screeneffect.h>
#include <prism/math.h>
#include <prism/sound.h>

#include "levelhandler.h"
#include "gamescreen.h"

static struct {
	MugenSpriteFile mSprites;
	MugenAnimations mAnimations;
	BlitzTimelineAnimations mTimelineAnimations;

	int mPlayerEntity;
	int mPlayerAnimationOffset;

	int mIsFadingOut;
} gData;

static void loadIntroScreen() {
	gData.mSprites = loadMugenSpriteFileWithoutPalette("assets/intro/INTRO.sff");
	gData.mAnimations = loadMugenAnimationFile("assets/intro/INTRO.air");
	gData.mTimelineAnimations = loadBlitzTimelineAnimations("assets/intro/INTRO.taf");

	int entityID;
	entityID = addBlitzEntity(makePosition(0, 0, 1));
	addBlitzMugenAnimationComponent(entityID, &gData.mSprites, &gData.mAnimations, 1);

	entityID = addBlitzEntity(makePosition(450, 400, 2));
	addBlitzMugenAnimationComponent(entityID, &gData.mSprites, &gData.mAnimations, 6);

	gData.mPlayerEntity = addBlitzEntity(makePosition(100, 460, 3));
	addBlitzMugenAnimationComponent(gData.mPlayerEntity, &gData.mSprites, &gData.mAnimations, 2);
	addBlitzPhysicsComponent(gData.mPlayerEntity);

	addBlitzTimelineComponent(getBlitzCameraHandlerEntityID(), &gData.mTimelineAnimations);

	gData.mPlayerAnimationOffset = 2;
	gData.mIsFadingOut = 0;
	playTrack(5);


	addFadeIn(30, NULL, NULL);
}

static void gotoGameScreen(void* tData) {
	(void)tData;
	resetLevelHandler();
	setLevelZoomOut();
	setNewScreen(&GameScreen);
}

static void updateWinningCondition() {
	Position pos = getBlitzEntityPosition(gData.mPlayerEntity);
	if (pos.x > 670) {
		playBlitzTimelineAnimation(getBlitzCameraHandlerEntityID(), 1);
		gData.mIsFadingOut = 1;
		addTimerCB(40, gotoGameScreen, NULL);
	}
}

static void updateIntroScreen() {
	if (gData.mIsFadingOut) return;

	Position* pos = getBlitzEntityPositionReference(gData.mPlayerEntity);
	pos->x = max(0, pos->x);

	if (hasPressedLeft()) {
		changeBlitzMugenAnimationIfDifferent(gData.mPlayerEntity, gData.mPlayerAnimationOffset + 1);
		setBlitzPhysicsVelocityX(gData.mPlayerEntity, -4);
		setMugenAnimationFaceDirection(getBlitzMugenAnimationID(gData.mPlayerEntity), 0);
	}
	if (hasPressedRight()) {
		changeBlitzMugenAnimationIfDifferent(gData.mPlayerEntity, gData.mPlayerAnimationOffset + 1);
		setBlitzPhysicsVelocityX(gData.mPlayerEntity, 4);
		setMugenAnimationFaceDirection(getBlitzMugenAnimationID(gData.mPlayerEntity), 1);
	}

	if (!hasPressedLeft() && !hasPressedRight()) {
		changeBlitzMugenAnimationIfDifferent(gData.mPlayerEntity, gData.mPlayerAnimationOffset);
		setBlitzPhysicsVelocityX(gData.mPlayerEntity, 0);
	}

	updateWinningCondition();
}

Screen IntroScreen = {
	.mLoad = loadIntroScreen,
	.mUpdate = updateIntroScreen,
};