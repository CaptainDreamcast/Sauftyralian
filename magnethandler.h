#pragma once

#include <prism/actorhandler.h>
#include <prism/geometry.h>

extern ActorBlueprint MagnetHandler;


void addSouthPole(Position tPos);
void addNorthPole(Position tPos);
Position getMagnetPosition(void* tEntry);