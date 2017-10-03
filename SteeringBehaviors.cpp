#include "2D/Transformations.h"
#include "misc/autolist.h"
#include "misc/utils.h"
#include "ParamLoader.h"
#include "PlayerBase.h"
#include "SoccerBall.h"
#include "SoccerTeam.h"
#include "SteeringBehaviors.h"

using std::string;
using std::vector;

SteeringBehaviors::SteeringBehaviors(PlayerBase* agent, SoccerPitch* world, SoccerBall ball):
	m_pPlayer(agent),m_iFlags(0),m_dMultSeparation(Prm.SeparationCoefficient),m_bTagged(false),m_dViewDistance(Prm.ViewDistance),m_pBall(ball),m_dInterposeDist(0.0),m_Antenna(5,Vector2D()){}

//------------------------------------AccumulateForce--------------------------------------
//
//This function calculates how much of its max steering force the vehicle has left to apply 
//and then applies that amount of the force to add.
//------------------------------------------------------------------------------------------
bool SteeringBehaviors::AccumulateForce(Vector2D &sf, Vector2D ForceToAdd) {

	//Calculate how much steering force the vehicle has used so far
	double MagnitudeSoFar = sf.Length();

	//Calculate how much steering force remains to be used by this vehicle
	double MagnitudeRemaining = m_pPlayer->MaxForce() - MagnitudeSoFar;

	//Return false if there is no more force left to use
	if (MagnitudeRemaining <= 0.0) return false;

	//Calculate the magnitude of the force we want to add
	double MagnitudeToAdd = ForceToAdd.Length();

	//Now calculate how much of the force we can really add
	if (MagnitudeToAdd > MagnitudeRemaining) MagnitudeToAdd = MagnitudeRemaining;

	//Add it to the steering force
	sf += (Vec2DNormalize(ForceToAdd) * MagnitudeToAdd);

	return true;

}

//----------------------------------------Calculate-----------------------------------------
//
// Calculates the overall steering force based on the currently active steering behaviors.
//
//------------------------------------------------------------------------------------------
Vector2D SteeringBehaviors::Calculate() {

	//Reset the force
	m_vSteeringForce.Zero();

	//This will hold the value of each individual steering force.
	m_vSteeringForce = SumForces();

	//Make sure the force doesn't exceed the players maximum allowable
	m_vSteeringForce.Truncate(m_pPlayer->MaxForce());

	return m_vSteeringForce;

}

//----------------------------------------SumForces-----------------------------------------
//
// This method calls each active steering behavior and accumulates their forces until the max
// steering force magnitude is reached at which time the function returns the steering force accumulated.
//
//------------------------------------------------------------------------------------------
Vector2D SteeringBehaviors::SumForces() {

	Vector2D force;

	//The soccer player must always tag their neighbors.
	FindNeighbours();

	if (On(separation)) {

		force += Separation() * m_dMultSeparation;
		if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;

	}

	if (On(seek)) {

		force += Seek(m_vTarget);
		if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;

	}

	if (On(arrive)) {

		force += Arrive(m_vTarget, fast);
		if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;

	}

	if (On(pursuit)) {

		force += Pursuit(m_pBall);
		if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;

	}

	if (On(interpose)) {

		force += Interpose(m_pBall, m_vTarget, m_dInterposeDist);
		if (!AccumulateForce(m_vSteeringForce, force)) return m_vSteeringForce;

	}

	return m_vSteeringForce;

}

//-------------------------------------ForwardComponent-------------------------------------
//
// Returns the forward component of the steering force
//
//------------------------------------------------------------------------------------------
double SteeringBehaviors::ForwardComponent() {
	return m_pPlayer->Heading().Dot(m_vSteeringForce);
}

//--------------------------------------SideComponent---------------------------------------
//
// Returns the side component of the steering force
//
//------------------------------------------------------------------------------------------
double SteeringBehaviors::SideComponent() {
	return m_pPlayer->Side().Dot(m_vSteeringForce) * m_pPlayer->MaxTurnRate();
}

//-----------------------------------------Seek--------------------------------------------
//
// Given a target, this behavior returns a steering force which will direct the agent towards the target
//
//-----------------------------------------------------------------------------------------
Vector2D SteeringBehaviors::Seek(Vector2D TargetPos) {

	Vector2D DesiredVelocity = Vec2DNormalize(TargetPos - m_pPlayer->Pos()) * m_pPlayer->MaxSpeed();
	return (DesiredVelocity - m_pPlayer->Velocity());

}

//---------------------------------------Arrive-------------------------------------------
//
// This behavior is similar to seek but it attempts to arrive at the target with a zero velocity
//
//-----------------------------------------------------------------------------------------
Vector2D SteeringBehaviors::Arrive(Vector2D TargetPos, Deceleration deceleration) {

	Vector2D ToTarget = TargetPos - m_pPlayer->Pos();

	//Calculate the distance to the target
	double dist = ToTarget.Length();
	if (dist > 0) {

		//Because Deceleration is enumerated as an int, this value is required to provide
		//fine tweaking of the deceleration.
		const double DecelerationTweaker = 0.3;

		//Calculate the speed required to reach the target given the desired deceleration
		double speed = dist / ((double)deceleration * DecelerationTweaker);

		//Make sure the velocity does not exceed the max
		speed = min(speed, m_pPlayer->MaxSpeed());

		//From here proceed just like Seek except we don't need to normalize the ToTarget
		//vector because we have already gone to the trouble of calculating its length: dist.
		Vector2D DesiredVelocity = ToTarget * speed / dist;
		return (DesiredVelocity - m_pPlayer->Velocity());

	}

	return Vector2D(0, 0);

}

//--------------------------------------Pursuit------------------------------------------
//
// This behavior creates a force that steers the agent towards the evader
//
//----------------------------------------------------------------------------------------
Vector2D SteeringBehaviors::Pursuit(const SoccerBall* ball) {

	Vector2D ToBall = ball->Pos() - m_pPlayer->Pos();

	//The lookaheadtime is proportional to the distance between the ball and the pursuer.
	double LookAheadTime = 0.0;

	if (ball->Speed() != 0.0) LookAheadTime = ToBall.Length() / ball->Speed();

	//Calculate where the ball will be at this time in the future.
	m_vTarget = ball->FuturePosition(LookAheadTime);

	//Now seek to the predicted future position of the ball.
	return Arrive(m_vTarget, fast);

}

//--------------------------------------Separation---------------------------------------
//
// Calculates a force repelling from the other neighbors
//
//----------------------------------------------------------------------------------------
Vector2D SteeringBehaviors::Separation() {

	//Iterate through all the neighbors and calculate the vector from.
	Vector2D SteeringForce;
	std::list<PlayerBase*>&AllPlayers = AutoList<PlayerBase>::GetAllMembers();
	std::list<PlayerBase*>::iterator curPlyr;

	for(curPlyr = AllPlayers.begin(); curPlyr != AllPlayers.end(); ++curPlyr){

		//Make sure this agent isn't included in the calculations and that the agent being examined is close enough.
		if ((*curPlyr != m_pPlayer) && (*curPlyr)->Steering()->Tagged()) {

			Vector2D ToAgent = m_pPlayer->Pos() - (*curPlyr)->Pos();

			//Scale the force inversely proportional to the agents distance from its neighbor.
			SteeringForce += Vec2DNormalize(ToAgent) / ToAgent.Length();

		}

	}

	return SteeringForce;

}

//--------------------------------------Interpose--------------------------------------
//
// Given an opponent and an object position this method returns a force that attempts 
// to position the agent between them.
//
//--------------------------------------------------------------------------------------
Vector2D SteeringBehaviors::Interpose(const SoccerBall* ball, Vector2D target, double DistFromTarget) {

	return Arrive(target + Vec2DNormalize(ball->Pos() - target) * DistFromTarget, normal);

}

//------------------------------------FindNeighbours------------------------------------
//
//Tags any players within a predefined radius
//
//--------------------------------------------------------------------------------------
void SteeringBehaviors::FindNeighbours() {

	std::list<PlayerBase*>&AllPlayers = AutoList<PlayerBase>::GetAllMembers();
	std::list<PlayerBase*>::iterator curPlyr;

	for (curPlyr = AllPlayers.begin(); curPlyr != AllPlayers.end(); ++curPlyr) {

		//First clear any current tag
		(*curPlyr)->Steering()->UnTag();

		//Work in distance squared to avoid sqrts
		Vector2D to = (*curPlyr)->Pos() - m_pPlayer->Pos();

		if (to.LengthSq() < (m_dViewDistance * m_dViewDistance)) (*curPlyr)->Steering()->Tag();

	}

}

//--------------------------------------RenderAids--------------------------------------
//
//--------------------------------------------------------------------------------------
void SteeringBehaviors::RenderAids() {

	//Render the steering force
	gdi->RedPen();
	gdi->Line(m_pPlayer->Pos(), m_pPlayer->Pos() + m_vSteeringForce * 20);

}