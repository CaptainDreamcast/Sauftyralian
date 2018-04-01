#include "levelhandler.h"

#include <prism/blitzmugenanimation.h>
#include <prism/blitzentity.h>
#include <prism/log.h>
#include <prism/system.h>
#include <prism/blitzcollision.h>
#include <prism/blitztimelineanimation.h>
#include <prism/blitzcamerahandler.h>

#include "collision.h"
#include "magnethandler.h"

static struct {
	MugenSpriteFile mSprites;
	MugenAnimations mAnimations;

	MugenSpriteFile mBGSprites;
	BlitzTimelineAnimations mTimelineAnimations;


	int mWidth;
	int mHeight;
	int mLevelEntityIDs[100][100];

	int mCurrentLevel;
	int mTileSet;

	int mHasPlayerStartTile;
	Vector3DI mPlayerStartTile;

	int mBackgroundEntity;

	CollisionData mBackgroundCollisionData;
	CollisionData mObstacleCollisionData;

	TextureData mWhiteTexture;

	int mIsZoomingOut;
} gData;

static void loadLevelTileFiles() {
	char path[1024];
	sprintf(path, "assets/tiles/%d.sff", gData.mTileSet);
	gData.mSprites = loadMugenSpriteFileWithoutPalette(path);

	sprintf(path, "assets/tiles/%d.air", gData.mTileSet);
	gData.mAnimations = loadMugenAnimationFile(path);
}

static void loadAnimationFromLevelTile(int i, int* oAnimation, int* oHorizontalMirror, int* oVerticalMirror) {
	if (i == 1) {
		*oAnimation = 1;
		*oHorizontalMirror = 0;
		*oVerticalMirror = 0;
	}
	else if (i < 6) {
		*oAnimation = 2;
		*oHorizontalMirror = (i == 3) || (i == 5);
		*oVerticalMirror = (i == 4) || (i == 5);
	}
	else if (i < 10) {
		*oAnimation = 6;
		*oHorizontalMirror = (i == 7) || (i == 9);
		*oVerticalMirror = (i == 8) || (i == 9);
	}
	else if (i < 11) {
		*oAnimation = 10;
		*oHorizontalMirror = 0;
		*oVerticalMirror = 0;
	}
	else if (i < 15) {
		*oAnimation = 11;
		*oHorizontalMirror = (i == 12) || (i == 14);
		*oVerticalMirror = (i == 13) || (i == 14);
	}
	else if (i < 16) {
		*oAnimation = 15;
		*oHorizontalMirror = 0;
		*oVerticalMirror = 0;
	}
	else if (i > 200 && i < 206) {
		*oAnimation = i;
		*oHorizontalMirror = 0;
		*oVerticalMirror = 0;
	}
	else {
		logWarning("Unimplemented tile.");
		logWarningInteger(i);
		*oAnimation = 1;
		*oHorizontalMirror = 0;
		*oVerticalMirror = 0;
	}

}

static void setCollisionFromIndex(int tEntityID, Position tPos, int tIndex, int tHorizontalMirror, int tVerticalMirror) {
	if (tIndex >= 204) return;

	addBlitzCollisionComponent(tEntityID);
	
	int collisionList;
	CollisionData* collisionData;
	int collisionID;
	if (tIndex < 100) {
		CollisionRect rect; 
		if (tHorizontalMirror && tVerticalMirror) rect = makeCollisionRect(makePosition(-32, -32, 0), makePosition(0, 0, 0));
		if (!tHorizontalMirror && tVerticalMirror) rect = makeCollisionRect(makePosition(0, -32, 0), makePosition(32, 0, 0));
		if (tHorizontalMirror && !tVerticalMirror) rect = makeCollisionRect(makePosition(-32, 0, 0), makePosition(0, 32, 0));
		if (!tHorizontalMirror && !tVerticalMirror) rect = makeCollisionRect(makePosition(0, 0, 0), makePosition(32, 32, 0));
		collisionList = getBackgroundCollisionList();
		collisionData = &gData.mBackgroundCollisionData;
		collisionID = addBlitzCollisionRect(tEntityID, collisionList, rect);

	}
	else if (tIndex < 202) {
		CollisionRect rect = makeCollisionRect(makePosition(5, 10, 0), makePosition(27, 27, 0));
		collisionList = getBackgroundObstacleList();
		collisionData = &gData.mObstacleCollisionData;
		collisionID = addBlitzCollisionRect(tEntityID, collisionList, rect);
	}
	else if (tIndex < 203) {
		CollisionRect rect = makeCollisionRect(makePosition(0, 0, 0), makePosition(32, 32, 0));
		collisionList = getBackgroundCollisionList();
		collisionData = &gData.mBackgroundCollisionData;
		collisionID = addBlitzCollisionRect(tEntityID, collisionList, rect);
		addNorthPole(vecAdd(tPos, makePosition(16, 16, 0)));
	}
	else if (tIndex < 204) {
		CollisionRect rect = makeCollisionRect(makePosition(0, 0, 0), makePosition(32, 32, 0));
		collisionList = getBackgroundCollisionList();
		collisionData = &gData.mBackgroundCollisionData;
		collisionID = addBlitzCollisionRect(tEntityID, collisionList, rect);
		addSouthPole(vecAdd(tPos, makePosition(16, 16, 0)));
	}
	else {
		logErrorFormat("Unimplemented index: %d.", tIndex);
		abortSystem();
		collisionID = -1;
	}

	setBlitzCollisionCollisionData(tEntityID, collisionID, collisionData);
	setBlitzCollisionSolid(tEntityID, collisionID, 0);
}

static void loadLevelTiles() {
	
	char path[1024];
	sprintf(path, "assets/levels/%d.txt", gData.mCurrentLevel);
	Buffer b = fileToBuffer(path);
	BufferPointer p = getBufferPointer(b);

	gData.mWidth = readIntegerFromTextStreamBufferPointer(&p);
	gData.mHeight = readIntegerFromTextStreamBufferPointer(&p);
	gData.mTileSet = readIntegerFromTextStreamBufferPointer(&p);

	loadLevelTileFiles();

	Position startPosition = makePosition(0, 480 - gData.mHeight, 5);
	int tileSizeX = gData.mWidth / 32;
	int tileSizeY = gData.mHeight / 32;
	int y, x;
	for (y = 0; y < tileSizeY; y++) {
		for (x = 0; x < tileSizeX; x++) {
			int index = readIntegerFromTextStreamBufferPointer(&p);
			if (!index) continue;
			if (index == -1) {
				gData.mHasPlayerStartTile = 1;
				gData.mPlayerStartTile = makeVector3DI(x, y, 0);
				continue;
			}

			int animation, horizontalMirror, verticalMirror;
			loadAnimationFromLevelTile(index, &animation, &horizontalMirror, &verticalMirror);
			Position pos = vecAdd(startPosition, makePosition((x + horizontalMirror) * 32, (y + verticalMirror) * 32, 0));
			gData.mLevelEntityIDs[y][x] = addBlitzEntity(pos);
			addBlitzMugenAnimationComponent(gData.mLevelEntityIDs[y][x], &gData.mSprites, &gData.mAnimations, animation);
			setMugenAnimationFaceDirection(getBlitzMugenAnimationID(gData.mLevelEntityIDs[y][x]), !horizontalMirror);
			setMugenAnimationVerticalFaceDirection(getBlitzMugenAnimationID(gData.mLevelEntityIDs[y][x]), !verticalMirror);
			setCollisionFromIndex(gData.mLevelEntityIDs[y][x], pos, animation, horizontalMirror, verticalMirror);
		}
	}

	freeBuffer(b);

	if(!gData.mHasPlayerStartTile) {
		logWarning("Unable to find player start tile.");
	}
}

static void loadLevelBackground() {
	char path[1024];
	sprintf(path, "assets/bgs/%d.sff", gData.mCurrentLevel);
	gData.mBGSprites = loadMugenSpriteFileWithoutPalette(path);
	
	gData.mBackgroundEntity = addBlitzEntity(makePosition(0, 480 - gData.mHeight, 1));
	addBlitzMugenAnimationComponentStatic(gData.mBackgroundEntity, &gData.mBGSprites, 1, 0);

	gData.mWhiteTexture = createWhiteTexture();
	int animationID = playOneFrameAnimationLoop(makePosition(0, 0, 2), &gData.mWhiteTexture);
	setAnimationSize(animationID, makePosition(640, 480, 1), makePosition(0, 0, 0));
	setAnimationColorType(animationID, COLOR_BLACK);
	setAnimationTransparency(animationID, 0.3);
}

static void loadLevelTimelineAnimationAndCamera() {
	gData.mTimelineAnimations = loadBlitzTimelineAnimations("assets/levels/GAME.taf");
	addBlitzTimelineComponent(getBlitzCameraHandlerEntityID(), &gData.mTimelineAnimations);

	if (gData.mIsZoomingOut) {
		playBlitzTimelineAnimation(getBlitzCameraHandlerEntityID(), 1);
		gData.mIsZoomingOut = 0;
	}
}

static void loadCollisionData() {
	gData.mBackgroundCollisionData.mList = getBackgroundCollisionList();
	gData.mObstacleCollisionData.mList = getBackgroundObstacleList();
}

static void loadLevelHandler(void* tData) {
	(void)tData;

	gData.mCurrentLevel++;
	
	loadCollisionData();
	loadLevelTiles();
	loadLevelBackground();
	loadLevelTimelineAnimationAndCamera();

	setBlitzCameraHandlerRange(makeGeoRectangle(0, 480 - gData.mHeight, gData.mWidth, gData.mHeight));
}


ActorBlueprint LevelHandler = {
	.mLoad = loadLevelHandler,
};

Position getPlayerStartPosition()
{
	return makePosition(gData.mPlayerStartTile.x * 32 + 16, (480 - gData.mHeight) + gData.mPlayerStartTile.y * 32 + 31, 0);
}

void resetLevelHandler()
{
	gData.mCurrentLevel = -1;
}

void setLevel(int tLevel)
{
	gData.mCurrentLevel = tLevel - 1;
}

int getLevelWidth()
{
	return gData.mWidth;
}

int getLevelHeight()
{
	return gData.mHeight;
}

int getCurrentLevel()
{
	return gData.mCurrentLevel;
}

void setLevelZoomOut()
{
	gData.mIsZoomingOut = 1;
}

MugenSpriteFile * getTileSprites()
{
	return &gData.mSprites;
}

MugenAnimations * getTileAnimations()
{
	return &gData.mAnimations;
}

BlitzTimelineAnimations * getGameTimelineAnimations()
{
	return &gData.mTimelineAnimations;
}
