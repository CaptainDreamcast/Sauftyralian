#include "titlescreen.h"

#include <prism/blitzmugenanimation.h>
#include <prism/blitzentity.h>
#include <prism/input.h>
#include <prism/blitztimelineanimation.h>
#include <prism/blitzcamerahandler.h>
#include <prism/timer.h>
#include <prism/sound.h>
#include <prism/mugensoundfilereader.h>
#include <prism/screeneffect.h>

#include "introscreen.h"

static struct {
	MugenSpriteFile mSprites;
	BlitzTimelineAnimations mTimelines;
	MugenSounds mSounds;

	int mLeftRollID;
	int mRightRollID;

	int mBGID;

	int mTitleID;

	int mHasPressedStart;
	int mPlayFadingInAnimations;
	int mIsFadingIn;
} gData;

static void stopFadingIn(void* tData);

static void loadTitle() {
	stopTrack();
	gData.mSprites = loadMugenSpriteFileWithoutPalette("assets/title/TITLE.sff");
	gData.mTimelines = loadBlitzTimelineAnimations("assets/title/TITLE.taf");
	gData.mSounds = loadMugenSoundFile("assets/title/TITLE.snd");

	gData.mLeftRollID = addBlitzEntity(makePosition(320, 0, 1));
	addBlitzMugenAnimationComponentStatic(gData.mLeftRollID, &gData.mSprites, 3, 0);
	addBlitzTimelineComponent(gData.mLeftRollID, &gData.mTimelines);
	
	gData.mRightRollID = addBlitzEntity(makePosition(320, 0, 1));
	addBlitzMugenAnimationComponentStatic(gData.mRightRollID, &gData.mSprites, 3, 1);
	addBlitzTimelineComponent(gData.mRightRollID, &gData.mTimelines);


	gData.mBGID = addBlitzEntity(makePosition(0, 0, 10));
	addBlitzMugenAnimationComponentStatic(gData.mBGID, &gData.mSprites, 2, 0);


	gData.mTitleID = addBlitzEntity(makePosition(320, 200, 50));
	addBlitzMugenAnimationComponentStatic(gData.mTitleID, &gData.mSprites, 1, 0);
	addBlitzTimelineComponent(gData.mTitleID, &gData.mTimelines);

	addBlitzTimelineComponent(getBlitzCameraHandlerEntityID(), &gData.mTimelines);
	if (gData.mPlayFadingInAnimations) {
		playBlitzTimelineAnimation(gData.mLeftRollID, 11);
		playBlitzTimelineAnimation(gData.mRightRollID, 12);
		playBlitzTimelineAnimation(gData.mTitleID, 13);
		playBlitzTimelineAnimation(getBlitzCameraHandlerEntityID(), 14);
		gData.mPlayFadingInAnimations = 0;
		playMugenSound(&gData.mSounds, 1, 0);
		addTimerCB(100, stopFadingIn, NULL);
		gData.mIsFadingIn = 1;
	}
	else {
		gData.mIsFadingIn = 0;
		addFadeIn(30, NULL, NULL);
	}

	gData.mHasPressedStart = 0;
	setWrapperTitleScreen(&TitleScreen);
	playTrack(4);
}

static void stopFadingIn(void* tData) {
	gData.mIsFadingIn = 0;
}

static void gotoIntroScreen(void* tData) {
	setNewScreen(&IntroScreen);
}

static void updateTitle() {
	if (gData.mHasPressedStart || gData.mIsFadingIn) return;

	if (hasPressedStartFlank()) {
		stopTrack();
		playBlitzTimelineAnimation(gData.mLeftRollID, 1);
		playBlitzTimelineAnimation(gData.mRightRollID, 2);
		playBlitzTimelineAnimation(gData.mTitleID, 3);
		playBlitzTimelineAnimation(getBlitzCameraHandlerEntityID(), 4);
		playMugenSound(&gData.mSounds, 1, 0);
		addTimerCB(190, gotoIntroScreen, NULL);

		gData.mHasPressedStart = 1;
	}
}

Screen TitleScreen = {
	.mLoad = loadTitle,
	.mUpdate = updateTitle,
};

void setTitleFadeIn()
{
	gData.mPlayFadingInAnimations = 1;
}
