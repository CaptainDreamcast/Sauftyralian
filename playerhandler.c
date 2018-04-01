#include "playerhandler.h"

#include <prism/blitzmugenanimation.h>
#include <prism/blitzentity.h>
#include <prism/blitzphysics.h>
#include <prism/input.h>
#include <prism/blitzcollision.h>
#include <prism/blitzparticles.h>
#include <prism/blitzcamerahandler.h>
#include <prism/timer.h>
#include <prism/blitztimelineanimation.h>
#include <prism/math.h>
#include <prism/mugensoundfilereader.h>

#include "levelhandler.h"
#include "collision.h"
#include "gamescreen.h"
#include "magnethandler.h"
#include "titlescreen.h"
#include "outroscreen.h"

static struct {
	MugenSpriteFile mSprites;
	MugenAnimations mAnimations;
	MugenSounds mSounds;

	int mPlayerEntity;
	int mIsNorth;
	int mAnimationOffset;

	int mAirjumpCounter;
	int mHasWon;

	CollisionData mCollisionData;
} gData;

static void playerHitCB(void* tCaller, void* tCollisionData);

static void loadPlayerHandler(void* tData) {
	(void)tData;

	gData.mSprites = loadMugenSpriteFileWithoutPalette("assets/player/PLAYER.sff");
	gData.mAnimations = loadMugenAnimationFile("assets/player/PLAYER.air");
	gData.mSounds = loadMugenSoundFile("assets/player/PLAYER.snd");

	Position pos = getPlayerStartPosition();
	pos.z = 10;
	gData.mPlayerEntity = addBlitzEntity(pos);
	addBlitzMugenAnimationComponent(gData.mPlayerEntity, &gData.mSprites, &gData.mAnimations, 1);
	addBlitzPhysicsComponent(gData.mPlayerEntity);
	setBlitzPhysicsGravity(gData.mPlayerEntity, makePosition(0, 0.2, 0));
	setBlitzPhysicsDragFactorOnCollision(gData.mPlayerEntity, makePosition(0.2, 0, 0));


	gData.mCollisionData.mList = getPlayerCollisionList();
	addBlitzCollisionComponent(gData.mPlayerEntity);
	int collisionID = addBlitzCollisionRect(gData.mPlayerEntity, getPlayerCollisionList(), makeCollisionRect(makePosition(-8, -32, 0), makePosition(8, 0, 0)));
	setBlitzCollisionSolid(gData.mPlayerEntity, collisionID, 1);
	setBlitzCollisionCollisionData(gData.mPlayerEntity, collisionID, &gData.mCollisionData);
	addBlitzCollisionCB(gData.mPlayerEntity, collisionID, playerHitCB, NULL);

	gData.mHasWon = 0;
	gData.mIsNorth = 0;
	gData.mAnimationOffset = 0;
	gData.mAirjumpCounter = 0;
}

static void addBloodParticles() {
	Position pos = vecSub(getBlitzEntityPosition(gData.mPlayerEntity), makePosition(0, 16, 0));
	pos.z = 11;
	addBlitzParticles(5, pos, makePosition(5, 5, 0), 2, 0.5, PARTICLE_ANGLE_UP, M_PI / 2, makePosition(0, 0.1, 0), makePosition(0.8, 0, 0), makePosition(0.2, 0, 0), 60, 10);
}

static void die() {
	addBloodParticles();
	playMugenSound(&gData.mSounds, 2, 0);

	Position pos = getPlayerStartPosition();
	pos.z = 10;
	setBlitzEntityPosition(gData.mPlayerEntity, pos);
	setBlitzPhysicsVelocity(gData.mPlayerEntity, makePosition(0, 0, 0));
}

static void handleMagnetInfluence(CollisionData* e, double tDirectionFactor) {
	Position magnetPos = getMagnetPosition(e->mData);
	Position playerPos = vecSub(getBlitzEntityPosition(gData.mPlayerEntity), makePosition(0, 16, 0));
	Position direction = vecSub(magnetPos, playerPos);
	direction.z = 0;
	double length = vecLength(direction);
	double strength = (0.5*(1 - (length / 100))) * tDirectionFactor;

	direction = vecNormalize(direction);
	direction = vecScale(direction, strength);
	addBlitzPhysicsImpulse(gData.mPlayerEntity, direction);
}

static void handleNorthInfluence(CollisionData* e) {
	handleMagnetInfluence(e, gData.mIsNorth ? -1 : 1);
}

static void handleSouthInfluence(CollisionData* e) {
	handleMagnetInfluence(e, gData.mIsNorth ? 1 : -1);
}


static void playerHitCB(void* tCaller, void* tCollisionData) {
	(void)tCaller;

	CollisionData* collisionData = tCollisionData;
	int listID = collisionData->mList;
	if (listID == getBackgroundObstacleList()) {
		die();
	}

	if (listID == getNorthPoleCollisionList()) {
		handleNorthInfluence(collisionData);
	}

	if (listID == getSouthPoleCollisionList()) {
		handleSouthInfluence(collisionData);
	}

}

static void addJumpDust() {
	Position pos = getBlitzEntityPosition(gData.mPlayerEntity);
	pos.z = 11;
	addBlitzParticles(2, pos, makePosition(1, 1, 0), 1, 0.25, PARTICLE_ANGLE_UP, M_PI / 2, makePosition(0., 0.1, 0), makePosition(0.7, 0.7, 0.7), makePosition(0.1, 0.1, 0.1), 10, 5);
}

static void updatePlayerAnimation() {
	if (getMugenAnimationAnimationNumber(getBlitzMugenAnimationID(gData.mPlayerEntity)) == gData.mAnimationOffset + 3 && hasBlitzCollidedBottom(gData.mPlayerEntity) && getBlitzPhysicsVelocityY(gData.mPlayerEntity) >= 0) {
		playMugenSound(&gData.mSounds, 1, 0);
		addJumpDust();
		gData.mAirjumpCounter = 0;
		changeBlitzMugenAnimation(gData.mPlayerEntity, gData.mAnimationOffset + 1);
	}
}

static int isJumping() {
	return getMugenAnimationAnimationNumber(getBlitzMugenAnimationID(gData.mPlayerEntity)) == gData.mAnimationOffset + 3;
}

static void switchPolarity() {
	int animationNumber = getMugenAnimationAnimationNumber(getBlitzMugenAnimationID(gData.mPlayerEntity)) % 4;
	int step = getMugenAnimationAnimationStep(getBlitzMugenAnimationID(gData.mPlayerEntity));
	
	gData.mIsNorth ^= 1;
	gData.mAnimationOffset = gData.mIsNorth ? 4 : 0;

	changeBlitzMugenAnimationWithStartStep(gData.mPlayerEntity, gData.mAnimationOffset + animationNumber, step);
}

static void updatePlayerInput() {
	double speed;
	if (isJumping()) speed = 3;
	else speed = 3;

	if (hasPressedLeft()) {
		if (getMugenAnimationAnimationNumber(getBlitzMugenAnimationID(gData.mPlayerEntity)) == gData.mAnimationOffset+1) changeBlitzMugenAnimation(gData.mPlayerEntity, gData.mAnimationOffset + 2);
		setMugenAnimationFaceDirection(getBlitzMugenAnimationID(gData.mPlayerEntity), 0);
		setBlitzPhysicsVelocityX(gData.mPlayerEntity, -speed);
	}
	if (hasPressedRight()) {
		if (getMugenAnimationAnimationNumber(getBlitzMugenAnimationID(gData.mPlayerEntity)) == gData.mAnimationOffset + 1) changeBlitzMugenAnimation(gData.mPlayerEntity, gData.mAnimationOffset + 2);
		setMugenAnimationFaceDirection(getBlitzMugenAnimationID(gData.mPlayerEntity), 1);
		setBlitzPhysicsVelocityX(gData.mPlayerEntity, speed);
	}

	if (!hasPressedLeft() && !hasPressedRight()) {
		if (getMugenAnimationAnimationNumber(getBlitzMugenAnimationID(gData.mPlayerEntity)) == gData.mAnimationOffset + 2) changeBlitzMugenAnimation(gData.mPlayerEntity, gData.mAnimationOffset + 1);
		setBlitzPhysicsVelocityX(gData.mPlayerEntity, 0);
	}

	if (hasPressedAFlank() && gData.mAirjumpCounter < 2) {
		if (getMugenAnimationAnimationNumber(getBlitzMugenAnimationID(gData.mPlayerEntity)) == gData.mAnimationOffset + 3) {
			gData.mAirjumpCounter++;
			addJumpDust();
		}
		playMugenSound(&gData.mSounds, 1, 0);
		changeBlitzMugenAnimation(gData.mPlayerEntity, gData.mAnimationOffset + 3);
		setBlitzPhysicsVelocityY(gData.mPlayerEntity, -3);
	}

	if (hasPressedBFlank()) {
		switchPolarity();
	}
	
	if (hasPressedRFlank()) {
		die();
	}

}

static void constraintPlayerPosition() {
	Position* p = getBlitzEntityPositionReference(gData.mPlayerEntity);
	p->x = max(0, p->x);
	p->y = max(480-getLevelHeight() + 16, p->y);
}

static void gotoOutroScreen(void* tCaller) {
	setNewScreen(&OutroScreen);
}

static void updateWinCondition() {
	if (gData.mHasWon) return;

	Position pos = getBlitzEntityPosition(gData.mPlayerEntity);
	if (pos.x > getLevelWidth()) {
		if (getCurrentLevel() != 9) {
			setNewScreen(&GameScreen);
		}
		else {
			playBlitzTimelineAnimation(getBlitzCameraHandlerEntityID(), 2);
			addTimerCB(40, gotoOutroScreen, NULL);
		}

		gData.mHasWon = 1;
	}
}

static void updateLoseCondition() {
	Position pos = getBlitzEntityPosition(gData.mPlayerEntity);
	if (pos.y > 480 + 64) {
		die();
	}
}

static void updatePlayerGravity() {
	Position vel = getBlitzPhysicsVelocity(gData.mPlayerEntity);
	if(vel.y > 0) setBlitzPhysicsGravity(gData.mPlayerEntity, makePosition(0, 0.2, 0));
	else setBlitzPhysicsGravity(gData.mPlayerEntity, makePosition(0, 0.1, 0));
}

static void updatePlayerCamera() {
	Position pos = getBlitzEntityPosition(gData.mPlayerEntity);
	setBlitzCameraPositionBasedOnCenterPoint(vecSub(pos, makePosition(0, 16, 0)));
}

static void updatePlayerHandler(void* tData) {
	(void)tData;
	updatePlayerAnimation();
	updatePlayerInput();
	constraintPlayerPosition();
	updateWinCondition();
	updateLoseCondition();
	updatePlayerGravity();
	updatePlayerCamera();
}

ActorBlueprint PlayerHandler = {
	.mLoad = loadPlayerHandler,
	.mUpdate = updatePlayerHandler,
};