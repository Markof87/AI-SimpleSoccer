#include "2D/geometry.h"
#include "2D/Transformations.h"
#include "Debug/DebugConsole.h"
#include "Game/EntityManager.h"
#include "Game/Region.h"
#include "misc/FrameCounter.h"

#include "Goal.h"
#include "ParamLoader.h"
#include "PlayerBase.h"
#include "SoccerBall.h"
#include "SoccerPitch.h"
#include "SoccerTeam.h"
#include "TeamStates.h"

const int NumRegionsHorizontal = 6;
const int NumRegionsVertical = 3;