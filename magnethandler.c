#include "magnethandler.h"

#include <prism/datastructures.h>
#include <prism/memoryhandler.h>
#include <prism/blitzentity.h>
#include <prism/blitzmugenanimation.h>
#include <prism/blitzcollision.h>
#include <prism/blitztimelineanimation.h>

#include "levelhandler.h"
#include "collision.h"

typedef struct {
	int mIsNorthPole;
	int mEntityID;
	CollisionData mCollisionData;
} PoleEntry;

static struct {
	List mPoles;
} gData;

static void loadMagnetHandler(void* tData) {
	gData.mPoles = new_list();
}

ActorBlueprint MagnetHandler = {
	.mLoad = loadMagnetHandler,
};

static void addPole(Position tPos, int tIsNorthPole) {
	PoleEntry* e = allocMemory(sizeof(PoleEntry));
	e->mIsNorthPole = tIsNorthPole;
	tPos.z--;
	e->mEntityID = addBlitzEntity(tPos);
	addBlitzMugenAnimationComponent(e->mEntityID, getTileSprites(), getTileAnimations(), 1000 + tIsNorthPole);
	setMugenAnimationBaseDrawScale(getBlitzMugenAnimationID(e->mEntityID), 2);
	addBlitzCollisionComponent(e->mEntityID);
	int list = tIsNorthPole ? getNorthPoleCollisionList() : getSouthPoleCollisionList();
	int collision = addBlitzCollisionCirc(e->mEntityID, list, makeCollisionCirc(makePosition(0, 0, 0), 100));
	e->mCollisionData.mData = e;
	e->mCollisionData.mList = list;
	setBlitzCollisionCollisionData(e->mEntityID, collision, &e->mCollisionData);

	addBlitzTimelineComponent(e->mEntityID, getGameTimelineAnimations());
	playBlitzTimelineAnimation(e->mEntityID, 1000);
}

void addSouthPole(Position tPos)
{
	addPole(tPos, 0);
}

void addNorthPole(Position tPos)
{
	addPole(tPos, 1);
}

Position getMagnetPosition(void * tEntry)
{
	PoleEntry* e = tEntry;
	return getBlitzEntityPosition(e->mEntityID);
}
