#include <Windows.h>
#include "2D/geometry.h"
#include "Debug/DebugConsole.h"
#include "Game/EntityManager.h"
#include "Messaging/MessageDispatcher.h"
#include "misc/utils.h"
#include "FieldPlayer.h"
#include "Goal.h"
#include "GoalKeeper.h"
#include "GoalKeeperStates.h"
#include "ParamLoader.h"
#include "PlayerBase.h"
#include "SoccerMessages.h"
#include "SoccerPitch.h"
#include "SoccerTeam.h"
#include "SteeringBehaviors.h"
#include "TeamStates.h"

using std::vector;

SoccerTeam::SoccerTeam(Goal* home_goal, Goal* opponents_goal, SoccerPitch* pitch, team_color color) :
	m_pOpponentGoal(opponents_goal), m_pHomeGoal(home_goal), m_pOpponents(NULL), m_pPitch(pitch), m_Color(color), m_dDistSqToBallOfClosestPlayer(0.0), m_pSupportingPlayer(NULL), m_pReceivingPlayer(NULL), m_pControllingPlayer(NULL), m_pPlayerClosestToBall(NULL) {

	//Setup the state machine
	m_pStateMachine = new StateMachine<SoccerTeam>(this);
	m_pStateMachine->SetCurrentState(Defending::Instance());
	m_pStateMachine->SetPreviousState(Defending::Instance());
	m_pStateMachine->SetGlobalState(NULL);

	//Create the players and goalkeeper.
	CreatePlayers();

	//Set default steering behaviors.
	std::vector<PlayerBase*>::iterator it = m_Players.begin();
	for (it; it != m_Players.end(); ++it) (*it)->Steering()->SeparationOn();

	//Create the sweet spot calculator.
	m_pSupportSpotCalc = new SupportSpotCalculator(Prm.NumSupportSpotsX, Prm.NumSupportSpotsY, this);

}

SoccerTeam::~SoccerTeam() {

	delete m_pStateMachine;

	std::vector<PlayerBase*>::iterator it = m_Players.begin();
	for (it; it != m_Players.end(); ++it) delete *it;

	delete m_pSupportSpotCalc;

}

//----------------------------------------Update----------------------------------------
//
// Iterates through each player's update function and calculates frequently accessed info.
//---------------------------------------------------------------------------------------
void SoccerTeam::Update() {

	//This information is used frequently so it's more efficient to calculate it just once each frame.
	CalculateClosestPlayerToBall();

	//The team state machine switches between attack/defense behavior. It also handles the 'kick off' state
	//where a team must return to their kick off positions before the whistle is blown.
	m_pStateMachine->Update();

	//Now update each player.
	std::vector<PlayerBase*>::iterator it = m_Players.begin();
	for (it; it != m_Players.end(); ++it) (*it)->Update();

}

//-----------------------------CalculateClosestPlayerToBall------------------------------
//
// Sets m_iClosestPlayerToBall to the player closest to the ball
//---------------------------------------------------------------------------------------
void SoccerTeam::CalculateClosestPlayerToBall() {

	double ClosestSoFar = MaxFloat;
	std::vector<PlayerBase*>::iterator it = m_Players.begin();
	for (it; it != m_Players.end(); ++it) {

		//Calculate the dist. Use the squared value to avoid sqrt.
		double dist = Vec2DDistanceSq((*it)->Pos(), Pitch()->Ball()->Pos());

		//Keep a record of this value for each player.
		(*it)->SetDistSqToBall(dist);
		if (dist < ClosestSoFar) {

			ClosestSoFar = dist;
			m_pPlayerClosestToBall = *it;

		}

	}

	m_dDistSqToBallOfClosestPlayer = ClosestSoFar;

}

//----------------------------DetermineBestSupportingAttacker----------------------------
//
// Calculate the closest player to the SupportSpot.
//---------------------------------------------------------------------------------------
PlayerBase* SoccerTeam::DetermineBestSupportingAttacker() {



}