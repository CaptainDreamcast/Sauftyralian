#pragma once

typedef struct {
	int mList;
	void* mData;
} CollisionData;

void loadGameCollisions();

int getBackgroundCollisionList(); 
int getPlayerCollisionList();
int getBackgroundObstacleList();
int getNorthPoleCollisionList();
int getSouthPoleCollisionList();