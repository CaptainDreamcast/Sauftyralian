#pragma once

#include <prism/mugenanimationhandler.h>

#include <prism/actorhandler.h>
#include <prism/blitztimelineanimation.h>

extern ActorBlueprint LevelHandler;

Position getPlayerStartPosition();
void resetLevelHandler();
void setLevel(int tLevel);

int getLevelWidth();
int getLevelHeight();
int getCurrentLevel();
void setLevelZoomOut();

MugenSpriteFile* getTileSprites();
MugenAnimations* getTileAnimations();
BlitzTimelineAnimations* getGameTimelineAnimations();