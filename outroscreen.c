#include "outroscreen.h"

#include <prism/blitzentity.h>
#include <prism/blitzcamerahandler.h>
#include <prism/blitzmugenanimation.h>
#include <prism/blitztimelineanimation.h>
#include <prism/blitzphysics.h>
#include <prism/screeneffect.h>
#include <prism/input.h>
#include <prism/math.h>
#include <prism/sound.h>

#include "outroscreen2.h"

static struct {
	MugenSpriteFile mSprites;
	MugenAnimations mAnimations;
	BlitzTimelineAnimations mTimelineAnimations;

	int mPlayerEntity;
	int mIsFadingOut;
	int mPlayerAnimationOffset;
} gData;

static void loadOutroScreen() {
	gData.mSprites = loadMugenSpriteFileWithoutPalette("assets/outro/OUTRO.sff");
	gData.mAnimations = loadMugenAnimationFile("assets/outro/OUTRO.air");
	gData.mTimelineAnimations = loadBlitzTimelineAnimations("assets/outro/OUTRO.taf");

	int entityID = addBlitzEntity(makePosition(0, 0, 1));
	addBlitzMugenAnimationComponent(entityID, &gData.mSprites, &gData.mAnimations, 1);

	addBlitzTimelineComponent(getBlitzCameraHandlerEntityID(), &gData.mTimelineAnimations);
	playBlitzTimelineAnimation(getBlitzCameraHandlerEntityID(), 1);

	gData.mPlayerEntity = addBlitzEntity(makePosition(100, 460, 3));
	addBlitzMugenAnimationComponent(gData.mPlayerEntity, &gData.mSprites, &gData.mAnimations, 2);
	addBlitzPhysicsComponent(gData.mPlayerEntity);

	int i;
	for (i = 0; i < 5; i++) {
		entityID = addBlitzEntity(makePosition(-100, -100, 2));
		addBlitzMugenAnimationComponent(entityID, &gData.mSprites, &gData.mAnimations, 11);
		addBlitzTimelineComponent(entityID, &gData.mTimelineAnimations);
		playBlitzTimelineAnimation(entityID, 10 + i);
	}

	gData.mPlayerAnimationOffset = 2;
	gData.mIsFadingOut = 0;

	playTrack(6);
}

static void gotoNextScreen(void* tData) {
	setNewScreen(&OutroScreen2);
}

static void updateWinningCondition() {
	Position pos = getBlitzEntityPosition(gData.mPlayerEntity);
	if (pos.x > 320) {
		changeBlitzMugenAnimationIfDifferent(gData.mPlayerEntity, gData.mPlayerAnimationOffset);
		setBlitzPhysicsVelocityX(gData.mPlayerEntity, 0);
		addFadeOut(180, gotoNextScreen, NULL);
		gData.mIsFadingOut = 1;

	}
}
static void updateOutroScreen() {
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

Screen OutroScreen = {
	.mLoad = loadOutroScreen,
	.mUpdate = updateOutroScreen,
};