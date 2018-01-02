#ifndef STEERINGBEHAVIORS_H
#define STEERINGBEHAVIORS_H
#pragma warning(disable:4786)

//------------------------------------------------------------------------
//
//  Name: SteeringBehaviors.h
//
//  Desc: Class to encapsulate steering behaviors for a Vehicle
//
//------------------------------------------------------------------------
#include <vector>
#include <Windows.h>
#include <string>

#include "2D/Vector2D.h"


class PlayerBase;
class SoccerPitch;
class SoccerBall;
class CWall;
class CObstacle;

class SteeringBehaviors {

private:

	PlayerBase* m_pPlayer;
	SoccerBall* m_pBall;

	//The steering force created by the combined effect of all the selected behaviors
	Vector2D m_vSteeringForce;

	//The current target
	Vector2D m_vTarget;

	//The distance the player tries to interpose from the target.
	double m_dInterposeDist;

	//Multipliers.
	double m_dMultSeparation;

	//How far it "can see"
	double m_dViewDistance;

	//Binary flags to indicate whether or not a behavior should be active
	int m_iFlags;

	enum behavior_type {

		none = 0x0000,
		seek = 0x0001,
		arrive = 0x0002,
		separation = 0x0004,
		pursuit = 0x0008,
		interpose = 0x0010

	};

	//Used by group behaviors to tag neighbours
	bool m_bTagged;

	//Arrive makes use of these to determine how quickly a vehicle should decelerate to its target
	enum Deceleration { slow = 3, normal = 2, fast = 1 };

	//Moves the agents towards a target position
	Vector2D Seek(Vector2D TargetPos);

	//Similar to seek but it attempts to arrive at the target position with a zero velocity
	Vector2D Arrive(Vector2D TargetPos, Deceleration deceleration);

	//Predicts where an agent will be in time T and seeks towards the point to intercept it
	Vector2D Pursuit(const SoccerBall* ball);

	Vector2D Separation();

	//Results in a steering force that attempts to steer the vehicle to the centre of the vector connecting two moving agents
	Vector2D Interpose(const SoccerBall* ball, Vector2D pos, double DistFromTarget);

	//Find any neighbours within the view radius
	void FindNeighbours();

	//This function tests if a specific bit of m_iFlags is set
	bool On(behavior_type bt) { return (m_iFlags & bt) == bt; }

	bool AccumulateForce(Vector2D &sf, Vector2D ForceToAdd);

	Vector2D SumForces();

	//A vertex buffer to contain the feelers rqd for dribbling
	std::vector<Vector2D> m_Antenna;

public:
	SteeringBehaviors(PlayerBase* agent, SoccerPitch* world, SoccerBall* ball);
	//virtual ~SteeringBehaviors();

	//Calculates and sums the steering forces from any active behaviors
	Vector2D Calculate();

	//Calculates the component of the steering force that is parallel with the vehicle heading
	double ForwardComponent();

	//Calculates the component of the steering force that is perpendicular with the vehicle heading
	double SideComponent();

	Vector2D Force()const { return m_vSteeringForce; }

	//Renders visual aids and info for seeing how each behavior is calculated
	//void RenderInfo();
	void RenderAids();

	Vector2D Target()const { return m_vTarget; }
	void SetTarget(const Vector2D t) { m_vTarget = t; }

	double InterposeDistance()const { return m_dInterposeDist; }
	void SetInterposeDistance(double d) { m_dInterposeDist = d; }

	bool Tagged()const { return m_bTagged; }
	void Tag() { m_bTagged = true; }
	void UnTag() { m_bTagged = false; }

	void SeekOn() { m_iFlags != seek; }
	void ArriveOn() { m_iFlags != arrive; }
	void PursuitOn() { m_iFlags != pursuit; }
	void SeparationOn() { m_iFlags != separation; }
	void InterposeOn(double d) { m_iFlags != interpose; m_dInterposeDist = d; }


	void SeekOff() { if (On(seek)) m_iFlags ^= seek; }
	void ArriveOff() { if (On(arrive)) m_iFlags ^= arrive; }
	void PursuitOff() { if (On(pursuit)) m_iFlags ^= pursuit; }
	void SeparationOff() { if (On(separation)) m_iFlags ^= separation; }
	void InterposeOff() { if (On(interpose)) m_iFlags ^= interpose; }

	bool IsSeekOn() { return On(seek); }
	bool IsArriveOn() { return On(arrive); }
	bool IsPursuitOn() { return On(pursuit); }
	bool IsSeparationOn() { return On(separation); }
	bool IsInterposeOn() { return On(interpose); }


};

#endif // !STEERINGBEHAVIORS_H
