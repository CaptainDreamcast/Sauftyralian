#include "collision.h"

#include <prism/collisionhandler.h>

static struct {
	int mBackgroundList;
	int mPlayerList;
	int mBackgroundObstacleList;
	int mNorthPoleList;
	int mSouthPoleList;
} gData;

void loadGameCollisions()
{
	gData.mBackgroundList = addCollisionListToHandler();
	gData.mBackgroundObstacleList = addCollisionListToHandler();
	gData.mNorthPoleList = addCollisionListToHandler();
	gData.mSouthPoleList = addCollisionListToHandler();
	gData.mPlayerList = addCollisionListToHandler();

	addCollisionHandlerCheck(gData.mPlayerList, gData.mBackgroundList);
	addCollisionHandlerCheck(gData.mPlayerList, gData.mBackgroundObstacleList);
	addCollisionHandlerCheck(gData.mPlayerList, gData.mNorthPoleList);
	addCollisionHandlerCheck(gData.mPlayerList, gData.mSouthPoleList);
}

int getBackgroundCollisionList()
{
	return gData.mBackgroundList;
}

int getPlayerCollisionList()
{
	return gData.mPlayerList;
}

int getBackgroundObstacleList()
{
	return gData.mBackgroundObstacleList;
}

int getNorthPoleCollisionList()
{
	return gData.mNorthPoleList;
}

int getSouthPoleCollisionList()
{
	return gData.mSouthPoleList;
}
